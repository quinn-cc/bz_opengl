#include "player.hpp"
#include "engine/client_engine.hpp"
#include "engine/types.hpp"
#include "game.hpp"
#include <string>
#include "spdlog/spdlog.h"

Player::Player(Game &game, const std::string name) : game(game) {
    this->name = name;
    alive = true;
    canSpawn = true;
    grounded = false;
    lastJumpTime = TimeUtils::GetCurrentTime();
    jumpCooldown = TimeUtils::getDuration(0.1f);

    physicsId = game.engine.physics->createPlayer(glm::vec3(1.0f, 2.0f, 1.0f));

    ClientMsg_Init initMsg;
    strcpy(initMsg.name, this->name.c_str());
    game.engine.network->send<ClientMsg_Init>(initMsg);
}

Player::~Player() {
    game.engine.physics->destroy(physicsId);
}

glm::vec3 Player::getForwardVector() const {
    return game.engine.physics->getForwardVector(physicsId);
}

void Player::update() {
    grounded = game.engine.physics->isGrounded(physicsId, glm::vec3(1.0f, 2.0f, 1.0f));       
    
    if (grounded) {
        glm::vec2 movement(0.0f);
        if (game.getFocusState() == FOCUS_STATE_GAME)
            movement = game.engine.input->getInputState().movement;
        float speed = game.world->getSettings().playerSpeed;
        float turnSpeed = game.world->getSettings().playerTurnSpeed;

        glm::vec3 movementVector = game.engine.physics->getForwardVector(physicsId);
        movementVector *= movement.y * speed;
        movementVector.y = game.engine.physics->getVelocity(physicsId).y;

        game.engine.physics->setVelocity(physicsId, movementVector);

        game.engine.physics->setAngularVelocity(physicsId, glm::vec3(
            0.0f,
            -movement.x * turnSpeed,
            0.0f
        ));

        if (game.getFocusState() == FOCUS_STATE_GAME) {
            if (grounded && game.engine.input->getInputState().jump && TimeUtils::GetElapsedTime(lastJumpTime, TimeUtils::GetCurrentTime()) >= jumpCooldown) {
                glm::vec3 velocity = game.engine.physics->getVelocity(physicsId);
                velocity.y = game.world->getSettings().playerJumpSpeed;
                game.engine.physics->setVelocity(physicsId, velocity);
                lastJumpTime = TimeUtils::GetCurrentTime();
                grounded = false;
            }
        }
    }

    this->location.position = game.engine.physics->getPosition(physicsId);
    this->location.rotation = game.engine.physics->getRotation(physicsId);
    this->velocity = game.engine.physics->getVelocity(physicsId);

    if (glm::distance(this->lastLocation.position, this->location.position) > POSITION_UPDATE_THRESHOLD ||
        angleBetween(this->lastLocation.rotation, this->location.rotation) > ROTATION_UPDATE_THRESHOLD) {
        ClientMsg_Location locMsg;
        locMsg.location = this->location;
        game.engine.network->send<ClientMsg_Location>(locMsg);
        this->lastLocation = this->location;
    }

    game.engine.render->setCameraPosition(this->location.position + glm::vec3(0.0f, 0.0f, 0.0f));
    game.engine.render->setCameraRotation(this->location.rotation);
}