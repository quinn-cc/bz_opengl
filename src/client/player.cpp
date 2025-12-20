#include "player.hpp"
#include "engine/client_engine.hpp"
#include "engine/types.hpp"
#include "game.hpp"

Player::Player(Game &game) : game(game) {
    alive = true;
    canSpawn = true;
    grounded = false;
    lastJumpTime = TimeUtils::GetCurrentTime();
    jumpCooldown = TimeUtils::getDuration(0.1f);

    physicsId = game.engine.physics->createPlayer(glm::vec3(1.0f, 2.0f, 1.0f));
}

Player::~Player() {
    game.engine.physics->destroy(physicsId);
}

void Player::update() {
    grounded = game.engine.physics->isGrounded(physicsId, glm::vec3(1.0f, 2.0f, 1.0f));       
    
    if (grounded) {
        glm::vec2 movement = game.engine.input->getInputState().movement;
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

        if (game.engine.input->getInputState().jump&& TimeUtils::GetElapsedTime(lastJumpTime, TimeUtils::GetCurrentTime()) >= jumpCooldown) {
            glm::vec3 velocity = game.engine.physics->getVelocity(physicsId);
            velocity.y = game.world->getSettings().playerJumpSpeed;
            game.engine.physics->setVelocity(physicsId, velocity);
            lastJumpTime = TimeUtils::GetCurrentTime();
            grounded = false;
        }
    }

    glm::vec3 position = game.engine.physics->getPosition(physicsId);
    glm::quat rotation = game.engine.physics->getRotation(physicsId);

    this->location.position = position;
    this->location.rotation = rotation;

    game.engine.render->setCameraPosition(position + glm::vec3(0.0f, 0.0f, 0.0f));
    game.engine.render->setCameraRotation(rotation);
}