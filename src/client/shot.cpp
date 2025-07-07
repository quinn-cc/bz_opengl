#include "shot.hpp"
#include <glm/glm.hpp>
#include "renderer.hpp"

std::vector<Shot *> Shot::shots;

Shot::Shot(int ownerId, glm::vec3 position, glm::vec3 velocity) {
    this->position = position;
    this->velocity = velocity;
    this->ownerId = ownerId;
    shots.push_back(this);
}

void Shot::Update() {
    position = position + (velocity + Renderer::GetInstance().GetDeltaTime());    
}

glm::vec3 Shot::GetPosition() {
    return position;
}

glm::vec3 Shot::GetVelocity() {
    return velocity;
}