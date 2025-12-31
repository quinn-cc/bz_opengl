#include "world.hpp"
#include "game.hpp"
#include <string>
#include <miniz.h>

namespace fs = std::filesystem;

World::World(Game &game, std::string worldDir) : game(game), worldDir(worldDir) {
    physicsId = game.engine.physics->create(worldDir + "/world.glb", 0.0f);
    
    // World "name" will be the folder name (not including the path)
    size_t lastSlash = worldDir.find_last_of("/\\");
    if (lastSlash == std::string::npos) {
        name = worldDir;
    } else {
        name = worldDir.substr(lastSlash + 1);
    }
    spdlog::info("World::World: Loaded world '{}'", name);

    // Create a zip file of the directory "worldDir", and save it at the same location
    // with the same name but with .zip extension
    fs::path inputDir(worldDir);
    fs::path outputZip = inputDir;
    outputZip += ".zip";
    zipDirectory(inputDir, outputZip);

    // Read the manifest
    readManifest("data/default_manifest.json");
    //readManifest(worldDir + "/manifest.json");
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

void World::readManifest(const fs::path& manifestPath) {
    if (!fs::exists(manifestPath)) {
        spdlog::warn("World::readManifest: Manifest file not found: {}", manifestPath.string());
        return;
    }

    std::ifstream manifestFile(manifestPath);
    if (!manifestFile) {
        spdlog::error("World::readManifest: Failed to open manifest file: {}", manifestPath.string());
        return;
    }

    nlohmann::json manifestJson;
    try {
        manifestFile >> manifestJson;
    } catch (const std::exception &e) {
        spdlog::error("World::readManifest: Failed to parse manifest JSON: {}", e.what());
        return;
    }

    if (manifestJson.contains("settings") && manifestJson["settings"].is_object()) {
        for (auto& [key, value] : manifestJson["settings"].items()) {
            if (value.is_number_float()) {
                settings[key] = value.get<float>();
            }
        }
    }
}

void World::update() {
    // Listen for player connection
    if (auto *connMsg = game.engine.network->peekMessage<ClientMsg_Connection>()) {
        std::vector<std::byte> worldData = getData();

        // Create a memory chunk of size sizeof(ServerMsg_Init) + dataSize
        // by mallocating it
        std::vector<char> initData(sizeof(ServerMsg_Init) + static_cast<size_t>(worldData.size())); // No additional data for now
        ServerMsg_Init* initHeaderMsg = reinterpret_cast<ServerMsg_Init*>(initData.data());

        initHeaderMsg->clientId = connMsg->clientId;
        strcpy(initHeaderMsg->serverName, "server");
        initHeaderMsg->settings = settings;
        initHeaderMsg->dataSize = static_cast<uint32_t>(worldData.size());

        // Now put the getData in the data section
        
        memmove(initHeaderMsg->data, worldData.data(), worldData.size());

        game.engine.network->send<ServerMsg_Init>(connMsg->clientId, initHeaderMsg);

        spdlog::trace("World::update: Sent init message to client id {}", connMsg->clientId);
    }
}

void World::setSetting(std::string key, float value) {
    settings[key] = value;

    // Broadcast the setting change to all clients
    ServerMsg_WorldSettingChange msg;
    strcpy(msg.key, key.c_str());
    msg.value = value;
    game.engine.network->sendAll<ServerMsg_WorldSettingChange>(&msg);
}

float World::getSetting(std::string key) const {
    auto it = settings.find(key);
    if (it != settings.end()) {
        return it->second;
    } else {
        throw std::runtime_error("Key not found in World settings: " + key);
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