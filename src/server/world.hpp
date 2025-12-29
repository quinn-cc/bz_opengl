#pragma once
#include "engine/types.hpp"

class Game;

class World {
private:
    Game &game;
    WorldSettingsMap settings;
    physics_id physicsId;

public:
    World(Game &game, std::string worldDir);
    ~World();
    
    void setSetting(std::string key, float value);
    float getSetting(std::string key) const;
    Location getSpawnLocation() const;
};