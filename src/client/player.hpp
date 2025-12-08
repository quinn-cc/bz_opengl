#pragma once
#include <glm/glm.hpp>
#include <string>
#include "engine/common/geometry.hpp"

class Player {
private:
    std::string name;
    Location position;
    glm::vec3 velocity;
    bool alive;
    bool canSpawn;

public:
    Player() = default;
    ~Player() = default;

    void update();
};