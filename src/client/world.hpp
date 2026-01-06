#pragma once
#include <string>
#include "engine/types.hpp"
#include <nlohmann/json.hpp>

class Game;

class World {
private:
    Game &game;
    render_id renderId;
    physics_id physicsId;
    PlayerParameters defaultPlayerParams;
    bool initialized = false;

    std::string worldDir;
    nlohmann::json manifest;

    void loadManifest(const std::filesystem::path& manifestPath);

public:
    World(Game &game, std::string worldDir);
    ~World();

    void load(std::string worldPath);
    bool isInitialized() const;
    void update();
    std::filesystem::path getAssetPath(const std::string &assetName) const;
};