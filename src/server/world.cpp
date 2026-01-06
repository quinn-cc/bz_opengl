#include "world.hpp"
#include "game.hpp"
#include <string>
#include <miniz.h>

namespace fs = std::filesystem;

World::World(Game &game, std::string worldName, nlohmann::json settings, std::string worldDir) : game(game), name(worldName), settings(settings), worldDir(worldDir) {
    // Load config file from ../data/config.json
    nlohmann::json configJson;
    std::filesystem::path configPath = std::filesystem::path("../data/config.json");
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
                assetPaths[key] = value.get<std::string>();
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
        }
    } else {
        spdlog::warn("World::World: No 'defaultPlayerParameters' object found in config");
    }

    loadManifest(fs::path(worldDir) / "manifest.json");
    
    // World "name" will be the folder name (not including the path)
    size_t lastSlash = worldDir.find_last_of("/\\");
    if (lastSlash == std::string::npos) {
        name = worldDir;
    } else {
        name = worldDir.substr(lastSlash + 1);
    }
    spdlog::info("World::World: Loaded world '{}'", worldName);

    // Create a zip file of the directory "worldDir", and save it at the same location
    // with the same name but with .zip extension
    fs::path inputDir(worldDir);
    fs::path outputZip = inputDir;
    outputZip += ".zip";
    zipDirectory(inputDir, outputZip);

    physicsId = game.engine.physics->create(getAssetPath("world"), 0.0f);
}

World::~World() {
    game.engine.physics->destroy(physicsId);
}

void World::zipDirectory(const fs::path& inputDir, const fs::path& outputZip) {
    if (!fs::exists(inputDir) || !fs::is_directory(inputDir)) {
        throw std::runtime_error("Input is not a directory");
    }

    mz_zip_archive zip{};
    if (!mz_zip_writer_init_file(&zip, outputZip.string().c_str(), 0)) {
        throw std::runtime_error("Failed to create zip file");
    }

    try {
        for (const auto& entry : fs::recursive_directory_iterator(inputDir)) {
            if (!entry.is_regular_file()) continue;

            fs::path absPath = entry.path();
            fs::path relPath = fs::relative(absPath, inputDir);

            // ZIP requires forward slashes
            std::string zipPath = relPath.generic_string();

            if (!mz_zip_writer_add_file(
                    &zip,
                    zipPath.c_str(),
                    absPath.string().c_str(),
                    nullptr,
                    0,
                    MZ_DEFAULT_LEVEL)) {
                throw std::runtime_error("Failed to add file: " + zipPath);
            }
        }

        if (!mz_zip_writer_finalize_archive(&zip)) {
            throw std::runtime_error("Failed to finalize zip");
        }

        mz_zip_writer_end(&zip);
    } catch (...) {
        mz_zip_writer_end(&zip);
        throw;
    }
}

std::vector<std::byte> World::getData() {
    // Construct the zip path from the world name
    fs::path zipPath = fs::path(worldDir);
    zipPath += ".zip";

    if (!fs::exists(zipPath)) {
        throw std::runtime_error("World zip file not found: " + zipPath.string());
    }

    // Get file size
    auto fileSize = fs::file_size(zipPath);

    // Read the entire zip file into memory
    std::vector<std::byte> data(fileSize);
    std::ifstream file(zipPath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open zip file: " + zipPath.string());
    }

    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    if (!file) {
        throw std::runtime_error("Failed to read zip file: " + zipPath.string());
    }

    return data;
}

void World::loadManifest(const fs::path& manifestPath) {
    if (!fs::exists(manifestPath)) {
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
}

void World::update() {
    // Listen for player connection
    if (auto *connMsg = game.engine.network->peekMessage<ClientMsg_PlayerJoin>()) {
        std::vector<std::byte> worldData = getData();

        // Create a memory chunk of size sizeof(ServerMsg_Init) + dataSize
        // by mallocating it
        ServerMsg_Init initHeaderMsg;
        initHeaderMsg.clientId = connMsg->clientId;
        initHeaderMsg.serverName = "server";
        initHeaderMsg.defaultPlayerParams = defaultPlayerParams;
        initHeaderMsg.worldData = worldData;
        game.engine.network->send<ServerMsg_Init>(connMsg->clientId, &initHeaderMsg);

        spdlog::trace("World::update: Sent init message to client id {}", connMsg->clientId);
    }
}

std::string World::getAssetPath(const std::string &assetName) const {
    // Check if manifest has "assets" and if assetName exists in it
    if (manifest.contains("assets") && manifest["assets"].contains(assetName)) {
        std::string assetPathStr = manifest["assets"][assetName].get<std::string>();
        fs::path assetPath = fs::path(worldDir) / assetPathStr;
        return assetPath.string();;
    } else {
        spdlog::error("World::getAssetPath: Asset '{}' not found in manifest, using default path", assetName);
        return "";
    }
}

Location World::getSpawnLocation() const {
    // Make x random between -10 and 10 and make z random between -10 and 10
    // and make z random between 10 and 15, and y rotation random between 0 and 2pi
    float x = static_cast<float>(rand() % 2001 - 1000) / 100.0f;
    float z = static_cast<float>(rand() % 2001 - 1000) / 100.0f;
    float y = static_cast<float>(rand() % 501 + 1000) / 100.0f;
    float rotY = static_cast<float>(rand() % 6283) / 1000.0f;
    return Location{
        .position = glm::vec3(x, y, z),
        .rotation = glm::angleAxis(rotY, glm::vec3(0, 1, 0))
    };
}