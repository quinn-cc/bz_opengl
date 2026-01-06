#include "client/config_client.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include "spdlog/spdlog.h"

ClientConfig ClientConfig::Load(const std::string &path) {
    ClientConfig config;

    std::ifstream file(path);
    if (!file.is_open()) {
        spdlog::warn("ClientConfig::Load: Unable to open {}", path);
        return config;
    }

    try {
        nlohmann::json jsonConfig;
        file >> jsonConfig;

        if (auto it = jsonConfig.find("tankPath"); it != jsonConfig.end() && it->is_string()) {
            config.tankPath = it->get<std::string>();
        }

        if (auto it = jsonConfig.find("ServerLists"); it != jsonConfig.end() && it->is_array()) {
            for (const auto &entry : *it) {
                if (!entry.is_object()) {
                    continue;
                }

                ClientServerListSource source;
                if (auto nameIt = entry.find("name"); nameIt != entry.end() && nameIt->is_string()) {
                    source.name = nameIt->get<std::string>();
                }

                if (auto urlIt = entry.find("url"); urlIt != entry.end() && urlIt->is_string()) {
                    source.url = urlIt->get<std::string>();
                }

                if (!source.url.empty()) {
                    config.serverLists.push_back(source);
                } else {
                    spdlog::warn("ClientConfig::Load: Skipping server list entry without URL");
                }
            }
        }
    } catch (const std::exception &ex) {
        spdlog::warn("ClientConfig::Load: Failed to parse {}: {}", path, ex.what());
    }

    return config;
}
