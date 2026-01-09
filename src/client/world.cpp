#include "world.hpp"
#include "game.hpp"
#include "spdlog/spdlog.h"
#include "common/data_path_resolver.hpp"
#include <fstream>
#include <iostream>
#include <cstring>
#include <filesystem>
#include <map>
#include <miniz.h>
#include <vector>

namespace {
std::string LeafKey(const std::string &key) {
    const auto separator = key.find_last_of('.');
    return separator == std::string::npos ? key : key.substr(separator + 1);
}
}

void World::registerAssets(const nlohmann::json &source, const std::filesystem::path &baseDir) {
    if (!source.is_object()) {
        return;
    }

    const auto assetsIt = source.find("assets");
    if (assetsIt == source.end()) {
        return;
    }

    if (!assetsIt->is_object()) {
        spdlog::warn("World::registerAssets: 'assets' exists but is not an object; skipping");
        return;
    }

    std::map<std::string, std::filesystem::path> collected;
    bz::data::CollectAssetEntries(*assetsIt, baseDir, collected);

    for (const auto &[key, path] : collected) {
        assetPaths[key] = path;
        assetPaths[LeafKey(key)] = path;
    }
}

World::World(Game &game, std::string worldDir) : game(game), worldDir(worldDir) {
    const auto userConfigPath = bz::data::EnsureUserConfigFile("config.json");

    const std::vector<bz::data::ConfigLayerSpec> layerSpecs = {
        {"common/config.json", "data/common/config.json", spdlog::level::err, true},
        {"client/config.json", "data/client/config.json", spdlog::level::debug, false},
        {userConfigPath, "user config", spdlog::level::debug, false}
    };

    const auto layers = bz::data::LoadConfigLayers(layerSpecs);

    for (const auto &layer : layers) {
        registerAssets(layer.json, layer.baseDir);

        if (layer.json.contains("defaultPlayerParameters") && layer.json["defaultPlayerParameters"].is_object()) {
            for (const auto &[key, value] : layer.json["defaultPlayerParameters"].items()) {
                if (value.is_number()) {
                    defaultPlayerParams[key] = value.get<float>();
                    spdlog::debug("World::World: Loaded default player parameter '{}' = {}", key, value.dump());
                }
            }
        }
    }
}

World::~World() {
    game.engine.render->destroy(renderId);
    physics.destroy();
}

bool World::isInitialized() const {
    return initialized;
}

bool World::unzipFromMemory(const std::byte* data, size_t size, const std::string& destDir) {
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));

    if (!mz_zip_reader_init_mem(&zip, data, size, 0)) {
        spdlog::error("World::unzipFromMemory: Failed to open zip from memory");
        return false;
    }

    int numFiles = mz_zip_reader_get_num_files(&zip);

    for (int i = 0; i < numFiles; i++) {
        mz_zip_archive_file_stat fileStat;
        if (!mz_zip_reader_file_stat(&zip, i, &fileStat)) {
            spdlog::error("World::unzipFromMemory: Failed to get file stat for index {}", i);
            mz_zip_reader_end(&zip);
            return false;
        }

        std::string outPath = destDir + "/" + fileStat.m_filename;

        if (mz_zip_reader_is_file_a_directory(&zip, i)) {
            std::filesystem::create_directories(outPath);
        } else {
            std::filesystem::create_directories(std::filesystem::path(outPath).parent_path());

            if (!mz_zip_reader_extract_to_file(&zip, i, outPath.c_str(), 0)) {
                spdlog::error("World::unzipFromMemory: Failed to extract: {}", fileStat.m_filename);
                mz_zip_reader_end(&zip);
                return false;
            }
        }
    }

    mz_zip_reader_end(&zip);
    spdlog::info("World::unzipFromMemory: Unzipped {} files to {}", numFiles, destDir);
    return true;
}

void World::loadManifest(const std::filesystem::path& manifestPath) {
    if (!std::filesystem::exists(manifestPath)) {
        spdlog::warn("World::loadManifest: Manifest file not found: {}", manifestPath.string());
        return;
    }

    std::ifstream manifestFile(manifestPath);
    if (!manifestFile) {
        spdlog::error("World::loadManifest: Failed to open manifest file: {}", manifestPath.string());
        return;
    }

    try {
        manifestFile >> manifest;
    } catch (const std::exception &e) {
        spdlog::error("World::loadManifest: Failed to parse manifest JSON: {}", e.what());
        return;
    }

    // See if there is a "defaultPlayerParameters" key in the manifest
    if (manifest.contains("defaultPlayerParameters")) {
        nlohmann::json paramsJson = manifest["defaultPlayerParameters"];

        for (auto& [key, value] : paramsJson.items()) {
            if (value.is_number()) {
                defaultPlayerParams[key] = value.get<float>();
                spdlog::info("World::loadManifest: Loaded player parameter '{}' = {}", key, value.get<float>());
            }
        }
    }

    // Load asset paths from config
    registerAssets(manifest, std::filesystem::path(worldDir));
}

void World::update() {
    if (auto *initMsg = game.engine.network->peekMessage<ServerMsg_Init>()) {
        spdlog::trace("World::update: Received init message from server");
        // Merge server params into client defaults (server values override)
        for (const auto& [key, val] : initMsg->defaultPlayerParams) {
            defaultPlayerParams[key] = val;
        }
        playerId = initMsg->clientId;

        if (!initMsg->worldData.empty()) {
            std::filesystem::path downloadsDir;
            if (const auto endpoint = game.engine.network->getServerEndpoint()) {
                downloadsDir = bz::data::EnsureUserWorldDirectoryForServer(endpoint->host, endpoint->port);
            } else {
                spdlog::warn("World::update: Server endpoint unknown; falling back to shared world directory");
                downloadsDir = bz::data::EnsureUserWorldsDirectory();
            }

            worldDir = downloadsDir.string();

            // Write initMsg->worldData to a file in worldDir called "world.zip"
            std::filesystem::path worldZipPath = downloadsDir / "world.zip";
            std::ofstream worldZipFile(worldZipPath, std::ios::binary);
            if (!worldZipFile) {
                spdlog::error("World::update: Failed to open world.zip for writing: {}", worldZipPath.string());
                return;
            }

            worldZipFile.write(reinterpret_cast<const char*>(initMsg->worldData.data()), initMsg->worldData.size());
            worldZipFile.close();

            unzipFromMemory(initMsg->worldData.data(), initMsg->worldData.size(), worldDir);

            const auto worldConfigPath = downloadsDir / "config.json";
            if (std::filesystem::exists(worldConfigPath)) {
                std::ifstream worldConfigFile(worldConfigPath);
                if (!worldConfigFile) {
                    spdlog::error("World::update: Failed to open world config for reading: {}", worldConfigPath.string());
                } else {
                    try {
                        nlohmann::json worldConfigJson;
                        worldConfigFile >> worldConfigJson;

                        if (!worldConfigJson.is_object()) {
                            spdlog::warn("World::update: World config is not a JSON object: {}", worldConfigPath.string());
                        } else {
                            constexpr const char* worldConfigLabel = "world config";
                            if (!bz::data::MergeConfigLayer(worldConfigLabel, worldConfigJson, downloadsDir)) {
                                spdlog::warn("World::update: Failed to merge world config layer from {}", worldConfigPath.string());
                            } else {
                                registerAssets(worldConfigJson, downloadsDir);
                            }
                        }
                    } catch (const std::exception &e) {
                        spdlog::error("World::update: Failed to parse world config JSON {}: {}", worldConfigPath.string(), e.what());
                    }
                }
            } else {
                spdlog::warn("World::update: World config not found at {}", worldConfigPath.string());
            }

            loadManifest(downloadsDir / "manifest.json");
        } else {
            spdlog::debug("World::update: Received bundled world indication; skipping download");
        }

        renderId = game.engine.render->create(getAssetPath("world").string());
        physics = game.engine.physics->createStaticMesh(getAssetPath("world").string(), 0.0f);

        spdlog::info("World::update: World initialized from server");
        initialized = true;
        return;
    }
}

std::filesystem::path World::getAssetPath(const std::string &assetName) const {
    if (assetPaths.find(assetName) != assetPaths.end()) {
        return assetPaths.at(assetName);
    } else {
        spdlog::error("World::getAssetPath: Asset '{}' not found", assetName);
        return std::filesystem::path();
    }
}