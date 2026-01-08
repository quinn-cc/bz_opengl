#include "world.hpp"
#include "game.hpp"
#include "spdlog/spdlog.h"
#include "common/data_path_resolver.hpp"
#include <fstream>
#include <iostream>
#include <cstring>
#include <miniz.h>

World::World(Game &game, std::string worldDir) : game(game), worldDir(worldDir) {
    // Load config file from the detected data directory
    nlohmann::json configJson;
    std::filesystem::path configPath = bz::data::Resolve("config.json");
    if (std::filesystem::exists(configPath)) {
        std::ifstream configFile(configPath);
        if (configFile) {
            try {
                configFile >> configJson;
            } catch (const std::exception &e) {
                spdlog::error("World::World: Failed to parse config JSON: {}", e.what());
            }
        } else {
            spdlog::error("World::World: Failed to open config file: {}", configPath.string());
        }
    } else {
        spdlog::warn("World::World: Config file not found: {}", configPath.string());
    }

    // Load asset paths from config
    if (configJson.contains("assets") && configJson["assets"].is_object()) {
        for (auto& [key, value] : configJson["assets"].items()) {
            if (value.is_string()) {
                assetPaths[key] = bz::data::Resolve(value.get<std::string>());
            }
        }
    } else {
        spdlog::warn("World::World: No 'assets' object found in config");
    }

    // Load the default player parameters from config
    if (configJson.contains("defaultPlayerParameters") && configJson["defaultPlayerParameters"].is_object()) {
        nlohmann::json paramsJson = configJson["defaultPlayerParameters"];
        for (auto& [key, value] : paramsJson.items()) {
            if (value.is_number_float()) {
                defaultPlayerParams[key] = value.get<float>();
            }

            spdlog::debug("World::World: Loaded default player parameter '{}' = {}", key, value.dump());
        }
    } else {
        spdlog::warn("World::World: No 'defaultPlayerParameters' object found in config");
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
    if (manifest.contains("assets") && manifest["assets"].is_object()) {
        for (auto& [key, value] : manifest["assets"].items()) {
            if (value.is_string()) {
                assetPaths[key] = std::filesystem::path(worldDir) / value.get<std::string>();
            }
        }
    }
}

void World::update() {
    if (auto *initMsg = game.engine.network->peekMessage<ServerMsg_Init>()) {
        spdlog::trace("World::update: Received init message from server");
        // Merge server params into client defaults (server values override)
        for (const auto& [key, val] : initMsg->defaultPlayerParams) {
            defaultPlayerParams[key] = val;
        }
        playerId = initMsg->clientId;

        // Write initMsg->worldData to a file in worldDir called "world.zip"
        std::filesystem::path worldZipPath = std::filesystem::path(worldDir) / "world.zip";
        std::ofstream worldZipFile(worldZipPath, std::ios::binary);
        if (!worldZipFile) {
            spdlog::error("World::update: Failed to open world.zip for writing: {}", worldZipPath.string());
            return;
        }
        // Assuming initMsg->worldData is a pointer to a byte array and we know its size
        // Here we just write dummy data for illustration; replace with actual data handling
        worldZipFile.write(reinterpret_cast<const char*>(initMsg->worldData.data()), initMsg->worldData.size());
        worldZipFile.close();
        
        unzipFromMemory(initMsg->worldData.data(), initMsg->worldData.size(), worldDir);

        loadManifest(std::filesystem::path(worldDir) / "manifest.json");

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