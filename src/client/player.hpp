#pragma once
#include <glm/glm.hpp>
#include <string>
#include "engine/types.hpp"

#define POSITION_UPDATE_THRESHOLD 0.01f
#define ROTATION_UPDATE_THRESHOLD 0.01f

class Game;

class Player {
private:
    Game &game;

    physics_id physicsId;

    TimeUtils::time lastJumpTime;
    TimeUtils::duration jumpCooldown;

    std::string name;
    Location lastLocation;
    Location location;
    glm::vec3 velocity;
    bool alive;
    bool canSpawn;
    bool grounded;

public:
    Player(Game &game, const std::string name);
    ~Player();

    std::string getName() const { return name; }

    void update();
};