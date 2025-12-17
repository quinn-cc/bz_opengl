#pragma once
#include <glm/glm.hpp>
#include <string>
#include "engine/types.hpp"

class Game;

class Player {
private:
    Game &game;

    physics_id physicsId;
    render_id renderId;

    std::string name;
    Location location;
    glm::vec3 velocity;
    bool alive;
    bool canSpawn;

public:
    Player(Game &game);
    ~Player();

    void update();
};