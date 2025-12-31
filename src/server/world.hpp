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
    std::string name;
    Game &game;
    SettingsMap settings;
    physics_id physicsId;
    std::string worldDir;

    void zipDirectory(const fs::path& inputDir, const fs::path& outputZip);
    std::vector<std::byte> getData();
    void readManifest(const fs::path& manifestPath);

public:
    World(Game &game, std::string worldDir);
    ~World();

    void update();
    
    void setSetting(std::string key, float value);
    float getSetting(std::string key) const;
    


    Location getSpawnLocation() const;
};