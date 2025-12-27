#pragma once
#include "engine/types.hpp"

class Game;

class World {
private:
    Game &game;
    WorldSettingsMap settings;

public:
    World(Game &game);
    ~World() = default;
    
    void setSetting(std::string key, float value);
    float getSetting(std::string key) const;
};