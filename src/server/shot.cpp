#include "shot.hpp"
#include <glm/glm.hpp>

#define BULLET_SIZE 1

std::vector<Shot *> Shot::shots;

Shot::Shot(Client *owner, glm::vec3 position, glm::vec3 velocity) {
    this->position = position;
    this->velocity = velocity;
    this->owner = owner;
    shots.push_back(this);
}

void Shot::Update() {
    float dt = 0.5f;

    position = position + (velocity * dt);
}