#include "world.hpp"
#include "game.hpp"
#include "spdlog/spdlog.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <miniz.h>

World::World(Game &game, std::string worldDir) : game(game), worldDir(worldDir) {
    
}

World::~World() {
    game.engine.render->destroy(renderId);
    game.engine.physics->destroy(physicsId);
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
        if (paramsJson.contains("speed")) defaultPlayerParams.speed = paramsJson["speed"].get<float>();
        if (paramsJson.contains("jumpSpeed")) defaultPlayerParams.jumpSpeed = paramsJson["jumpSpeed"].get<float>();
        if (paramsJson.contains("turnSpeed")) defaultPlayerParams.turnSpeed = paramsJson["turnSpeed"].get<float>();
        if (paramsJson.contains("shotSpeed")) defaultPlayerParams.shotSpeed = paramsJson["shotSpeed"].get<float>();
        if (paramsJson.contains("gravity")) defaultPlayerParams.gravity = paramsJson["gravity"].get<float>();
        if (paramsJson.contains("forwardSpeedMultiplier")) defaultPlayerParams.forwardSpeedMultiplier = paramsJson["forwardSpeedMultiplier"].get<float>();
        if (paramsJson.contains("backwardSpeedMultiplier")) defaultPlayerParams.backwardSpeedMultiplier = paramsJson["backwardSpeedMultiplier"].get<float>();
        if (paramsJson.contains("leftTurnSpeedMultiplier")) defaultPlayerParams.leftTurnSpeedMultiplier = paramsJson["leftTurnSpeedMultiplier"].get<float>();
        if (paramsJson.contains("rightTurnSpeedMultiplier")) defaultPlayerParams.rightTurnSpeedMultiplier = paramsJson["rightTurnSpeedMultiplier"].get<float>();
    }
}

void World::update() {
    if (auto *initMsg = game.engine.network->peekMessage<ServerMsg_Init>()) {
        spdlog::trace("World::update: Received init message from server");
        // Load settings from init message
        defaultPlayerParams = initMsg->defaultPlayerParams;

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

        game.engine.physics->setGravity(defaultPlayerParams.gravity);

        renderId = game.engine.render->create(getAssetPath("world").string());
        physicsId = game.engine.physics->create(getAssetPath("world").string(), 0.0f);

        spdlog::info("World::update: World initialized from server");
        initialized = true;
        return;
    }
}

std::filesystem::path World::getAssetPath(const std::string &assetName) const {
    // Check if assetName exists in manifest "assets"
    if (manifest.contains("assets") && manifest["assets"].contains(assetName)) {
        std::string assetPathStr = manifest["assets"][assetName].get<std::string>();
        std::filesystem::path assetPath = std::filesystem::path(worldDir) / assetPathStr;
        return assetPath;
    } else {
        throw std::runtime_error("World::getAssetPath: Asset not found in manifest: " + assetName);
    }
}