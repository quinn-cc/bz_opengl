#include "client/server_list_fetcher.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "spdlog/spdlog.h"

#include <sstream>

namespace {
size_t CurlWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total = size * nmemb;
    auto *buffer = static_cast<std::string *>(userdata);
    buffer->append(ptr, total);
    return total;
}

int parseIntegerField(const nlohmann::json &object, const char *key) {
    if (!object.contains(key)) {
        return -1;
    }

    const auto &value = object.at(key);
    try {
        if (value.is_number_integer()) {
            return value.get<int>();
        }
        if (value.is_string()) {
            return std::stoi(value.get<std::string>());
        }
    } catch (...) {
        return -1;
    }

    return -1;
}
}

ServerListFetcher::ServerListFetcher(std::vector<ClientServerListSource> sources)
    : sources(std::move(sources)) {
    if (!this->sources.empty()) {
        if (curl_global_init(CURL_GLOBAL_DEFAULT) == 0) {
            curlInitialized = true;
        } else {
            spdlog::warn("ServerListFetcher: Failed to initialize cURL");
        }
    }
}

ServerListFetcher::~ServerListFetcher() {
    if (worker.joinable()) {
        worker.join();
    }

    if (curlInitialized) {
        curl_global_cleanup();
    }
}

void ServerListFetcher::requestRefresh() {
    if (sources.empty() || !curlInitialized) {
        return;
    }

    bool expected = false;
    if (!fetching.compare_exchange_strong(expected, true)) {
        return;
    }

    if (worker.joinable()) {
        worker.join();
    }

    worker = std::thread(&ServerListFetcher::workerProc, this);
}

std::vector<ServerListFetcher::ServerRecord> ServerListFetcher::getServers() const {
    std::lock_guard<std::mutex> lock(recordsMutex);
    return records;
}

std::size_t ServerListFetcher::getGeneration() const {
    return generation.load();
}

bool ServerListFetcher::isFetching() const {
    return fetching.load();
}

void ServerListFetcher::workerProc() {
    auto newRecords = fetchOnce();
    {
        std::lock_guard<std::mutex> lock(recordsMutex);
        records = std::move(newRecords);
        generation++;
    }
    fetching.store(false);
}

std::vector<ServerListFetcher::ServerRecord> ServerListFetcher::fetchOnce() const {
    std::vector<ServerRecord> combined;
    combined.reserve(16);

    for (const auto &source : sources) {
        std::string responseBody;
        if (!fetchUrl(source.url, responseBody)) {
            continue;
        }

        auto parsed = parseResponse(source, responseBody);
        combined.insert(combined.end(), parsed.begin(), parsed.end());
    }

    return combined;
}

bool ServerListFetcher::fetchUrl(const std::string &url, std::string &outBody) {
    CURL *curlHandle = curl_easy_init();
    if (!curlHandle) {
        spdlog::warn("ServerListFetcher: curl_easy_init failed");
        return false;
    }

    curl_easy_setopt(curlHandle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curlHandle, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &outBody);

    CURLcode result = curl_easy_perform(curlHandle);
    if (result != CURLE_OK) {
        spdlog::warn("ServerListFetcher: Request to {} failed: {}", url, curl_easy_strerror(result));
        curl_easy_cleanup(curlHandle);
        return false;
    }

    long status = 0;
    curl_easy_getinfo(curlHandle, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curlHandle);

    if (status < 200 || status >= 300) {
        spdlog::warn("ServerListFetcher: {} returned HTTP status {}", url, status);
        return false;
    }

    return true;
}

std::vector<ServerListFetcher::ServerRecord> ServerListFetcher::parseResponse(
    const ClientServerListSource &source,
    const std::string &body) {
    std::vector<ServerRecord> records;

    try {
        nlohmann::json jsonData = nlohmann::json::parse(body);
        if (!jsonData.contains("servers") || !jsonData["servers"].is_array()) {
            spdlog::warn("ServerListFetcher: Server list from {} missing 'servers' array", source.url);
            return records;
        }

        for (const auto &server : jsonData["servers"]) {
            if (!server.is_object()) {
                continue;
            }

            if (!server.contains("host")) {
                continue;
            }

            ServerRecord record;
            record.sourceName = source.name.empty() ? source.url : source.name;
            record.host = server.value("host", "");
            std::string portString = server.value("port", "1234");

            try {
                record.port = static_cast<uint16_t>(std::stoi(portString));
            } catch (...) {
                record.port = 1234;
            }

            record.name = server.value("name", record.host);
            record.gameMode = server.value("game_mode", std::string{});
            record.maxPlayers = parseIntegerField(server, "max_players");
            record.activePlayers = parseIntegerField(server, "active_players");

            if (record.host.empty()) {
                continue;
            }

            records.push_back(record);
        }
    } catch (const std::exception &ex) {
        spdlog::warn("ServerListFetcher: Failed to parse response from {}: {}", source.url, ex.what());
    }

    return records;
}
