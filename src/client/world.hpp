#pragma once
#include <string>
#include "engine/types.hpp"

class Game;

class World {
private:
    Game &game;
    render_id renderId;
    physics_id physicsId;
    SettingsMap settings;
    bool initialized = false;

    void setSetting(const std::string &key, float value);

public:
    World(Game &game);
    ~World();

    void load(std::string worldPath);
    bool isInitialized() const;
    float getSetting(const std::string &key) const;
    void update();
};