#pragma once
#include "engine/types.hpp"
#include "engine/physics/compound_body.hpp"
#include <filesystem>
#include <map>
#include <nlohmann/json.hpp>
#include <string>

namespace fs = std::filesystem;

class Game;

class World {
private:
    Game &game;

    std::string name;
    std::string worldDir;
    std::map<std::string, std::filesystem::path> assetPaths;
    nlohmann::json config;

    PlayerParameters defaultPlayerParams;
    PhysicsCompoundBody physics;
    bool zipWorldOnStartup = true;

    void zipDirectory(const fs::path& inputDir, const fs::path& outputZip);
    std::vector<std::byte> getData();

public:
    World(Game &game,
          std::string worldName,
          nlohmann::json worldConfig,
          std::string worldDir,
          bool enableWorldZipping);
    ~World();

    void update();
    
    std::string getAssetPath(const std::string &assetName) const;
    const nlohmann::json &getConfig() const { return config; }
    const PlayerParameters& getDefaultPlayerParameters() const { return defaultPlayerParams; }

    Location getSpawnLocation() const;
};