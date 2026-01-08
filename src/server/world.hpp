#pragma once
#include "engine/types.hpp"
#include "engine/physics/compound_body.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

class Game;

class World {
private:
    Game &game;

    std::string name;
    nlohmann::json settings;
    nlohmann::json manifest;
    std::string worldDir;
    std::map<std::string, std::filesystem::path> assetPaths;

    PlayerParameters defaultPlayerParams;
    PhysicsCompoundBody physics;

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