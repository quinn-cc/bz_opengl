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
    yaw = 0.0f;
    lastYaw = yaw;
    moveSpeed = 10.0f;
    turnSpeed = -2.0f;
}

void Player::Update() {
    if (Input::GetInstance().FireReady()) {
        float speed = 1;
        glm::vec3 velocity = glm::normalize(glm::vec3(
            sinf(yaw),
            0.0f,
            cosf(yaw)
        ));

        velocity *= speed;
        Shot *shot = new Shot(0, location.position, velocity);
        Networker::GetInstance().MsgSend_Shot(shot);
    }

    // Rotation
    glm::vec2 movement = Input::GetInstance().GetMovement();
    yaw += (turnSpeed * Renderer::GetInstance().GetDeltaTime()) * movement.x;
    // Forward/backward
    glm::vec3 forward = GetForwardVector();
    location.position = location.position + (movement.y * forward * moveSpeed * Renderer::GetInstance().GetDeltaTime());
    glm::mat4 mat = glm::rotate(glm::mat4(1.0f), yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    location.rotation = glm::quat_cast(mat);

    // Update location
    bool updateLoc = false;

    if (glm::distance(lastLocation.position, location.position) > POS_UPDATE_TOLERANCE) {
        updateLoc = true;
        lastLocation.position = location.position;
    }
    if (abs(lastYaw - yaw) > ROT_UPDATE_TOLERANCE) {
        updateLoc = true;
        lastYaw = yaw;
    }

    if (updateLoc) {
        spdlog::info("Updating my location x={},y={},z={}", location.position.x, location.position.y, location.position.z);
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
    return glm::normalize(glm::vec3(
        sinf(yaw),  // x
        0.0f,         // y (no vertical movement)
        cosf(yaw)   // z
    ));
}

Location Player::GetLocation() {
    return location;
}