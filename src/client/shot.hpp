#pragma once
#include <glm/glm.hpp>
#include <vector>

#define BULLET_SIZE 1

class Shot {
private:
    glm::vec3 position;
    glm::vec3 velocity;
    int ownerId;

public:
    static std::vector<Shot *> shots;

    Shot(int ownerId, glm::vec3 position, glm::vec3 velocity);
    void Update();
    glm::vec3 GetPosition();
    glm::vec3 GetVelocity();
};