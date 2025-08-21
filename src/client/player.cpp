#include "player.hpp"
#include <vector>
#include <spdlog/spdlog.h>
#include "shot.hpp"
#include "networker.hpp"
#include "input.hpp"
#include "renderer.hpp"

#define POS_UPDATE_TOLERANCE 0.1
#define ROT_UPDATE_TOLERANCE 0.05


Player &Player::GetInstance() {
    static Player instance;
    return instance;
}

void Player::Init() {
    location.position = { 0.0f, 0.5f, 0.0f };
    location.rotation = { 0.0f, 0.0f, 0.0f, 0.0f };
    lastLocation = location;
    moveSpeed = 10.0f;
    turnSpeed = -2.0f;
}

void Player::Update() {
    if (Input::GetInstance().FireReady()) {
        float speed = 1;
        glm::vec3 velocity = GetForwardVector() * speed;
        Shot *shot = new Shot(location.position, velocity);
        Networker::GetInstance().MsgSend_Shot(shot);
    }

    // Rotation
    // glm::vec2 movement = Input::GetInstance().GetMovement();
    // getYaw() += (turnSpeed * Renderer::GetInstance().GetDeltaTime()) * movement.x;
    // // Forward/backward
    // glm::vec3 forward = GetForwardVector();
    // location.position = location.position + (movement.y * forward * moveSpeed * Renderer::GetInstance().GetDeltaTime());
    // glm::mat4 mat = glm::rotate(glm::mat4(1.0f), getYaw(), glm::vec3(0.0f, 1.0f, 0.0f));
    // location.rotation = glm::quat_cast(mat);

    // Update location
    bool updateLoc = false;

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

    if (updateLoc) {
        Networker::GetInstance().MsgSend_Location();
    }
}

void Player::SetName(std::string name) {
    this->name = name;
}

std::string Player::GetName() {
    return name;
}

glm::vec3 Player::GetForwardVector() {
    return location.rotation * glm::vec3(0, 0, 1);
}

Location Player::GetLocation() {
    return location;
}

void Player::SetLocation(Location location) {
    this->location = location;
}