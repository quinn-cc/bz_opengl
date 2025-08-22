#include "player.hpp"
#include <vector>
#include <spdlog/spdlog.h>

#define POS_UPDATE_TOLERANCE 0.1
#define ROT_UPDATE_TOLERANCE 0.05

Player &Player::GetInstance() {
    static Player instance;
    return instance;
}

void Player::Init(std::string name) {
    this->name = name;
    location.position = { 0.0f, 0.5f, 0.0f };
    location.rotation = { 0.0f, 0.0f, 0.0f, 0.0f };
    lastLocation = location;
    moveSpeed = 10.0f;
    turnSpeed = -2.0f;
    updateLoc = true;
    alive = false;
    canSpawn = false;
}

void Player::Update() {
    updateLoc = false;

    if (glm::distance(lastLocation.position, location.position) > POS_UPDATE_TOLERANCE) {
        updateLoc = true;
        lastLocation.position = location.position;
    }

    float dot = glm::dot(lastLocation.rotation, location.rotation);
    float angle = 2.0f * acos(glm::clamp(dot, -1.0f, 1.0f));

    if (angle > ROT_UPDATE_TOLERANCE) {
        updateLoc = true;
        lastLocation.rotation = location.rotation;
    }
}

bool Player::LocationChanged() {
    return updateLoc;
}

void Player::Close() {
    // Cleanup if needed
}