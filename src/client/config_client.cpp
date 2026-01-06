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

        if (auto serverListsIt = jsonConfig.find("serverLists"); serverListsIt != jsonConfig.end()) {
            if (!serverListsIt->is_object()) {
                spdlog::warn("ClientConfig::Load: 'serverLists' must be an object");
            } else {
                const auto &serverListsObject = *serverListsIt;

                if (auto showLanIt = serverListsObject.find("showLAN"); showLanIt != serverListsObject.end() && showLanIt->is_boolean()) {
                    config.showLanServers = showLanIt->get<bool>();
                }

                if (auto defaultIt = serverListsObject.find("default"); defaultIt != serverListsObject.end() && defaultIt->is_string()) {
                    config.defaultServerList = defaultIt->get<std::string>();
                }

                if (auto sourcesIt = serverListsObject.find("sources"); sourcesIt != serverListsObject.end()) {
                    if (!sourcesIt->is_array()) {
                        spdlog::warn("ClientConfig::Load: 'sources' must be an array");
                    } else {
                        for (const auto &entry : *sourcesIt) {
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
                }
            }
        }
    } catch (const std::exception &ex) {
        spdlog::warn("ClientConfig::Load: Failed to parse {}: {}", path, ex.what());
    }

    return config;
}

bool ClientConfig::Save(const std::string &path) const {
    nlohmann::json jsonConfig;

    if (!tankPath.empty()) {
        jsonConfig["tankPath"] = tankPath;
    }

    nlohmann::json serverListsObject = nlohmann::json::object();
    serverListsObject["showLAN"] = showLanServers;
    if (!defaultServerList.empty()) {
        serverListsObject["default"] = defaultServerList;
    }

    nlohmann::json sourcesArray = nlohmann::json::array();
    for (const auto &source : serverLists) {
        if (source.url.empty()) {
            continue;
        }

        nlohmann::json entry;
        entry["url"] = source.url;
        if (!source.name.empty()) {
            entry["name"] = source.name;
        }
        sourcesArray.push_back(std::move(entry));
    }

    serverListsObject["sources"] = std::move(sourcesArray);
    jsonConfig["serverLists"] = std::move(serverListsObject);

    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open()) {
        spdlog::warn("ClientConfig::Save: Unable to open {} for writing", path);
        return false;
    }

    try {
        file << jsonConfig.dump(4) << '\n';
    } catch (const std::exception &ex) {
        spdlog::warn("ClientConfig::Save: Failed to write {}: {}", path, ex.what());
        return false;
    }

    return true;
}
