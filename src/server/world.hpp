#pragma once
#include "engine/types.hpp"
#include <filesystem>
#include <stdexcept>
#include <string>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

class Game;

class World {
private:
    Game &game;

    std::string name;
    nlohmann::json settings;
    nlohmann::json manifest;
    std::string worldDir;
    std::map<std::string, std::string> assetPaths;

    PlayerParameters defaultPlayerParams;
    physics_id physicsId;

    void zipDirectory(const fs::path& inputDir, const fs::path& outputZip);
    std::vector<std::byte> getData();
    void loadManifest(const fs::path& manifestPath);

public:
    World(Game &game, std::string worldName, nlohmann::json settings, std::string worldDir);
    ~World();

    void update();
    
    std::string getAssetPath(const std::string &assetName) const;
    nlohmann::json getManifest() const { return manifest; }
    const PlayerParameters& getDefaultPlayerParameters() const { return defaultPlayerParams; }

    Location getSpawnLocation() const;
};