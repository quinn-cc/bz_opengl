#pragma once
#include <string>
#include "engine/types.hpp"

class Game;

class World {
private:
    Game &game;
    render_id renderId;
    physics_id physicsId;
    PlayerParameters defaultPlayerParams;
    bool initialized = false;

public:
    World(Game &game);
    ~World();

    void load(std::string worldPath);
    bool isInitialized() const;
    void update();
};