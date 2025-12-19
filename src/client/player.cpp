#include "player.hpp"
#include "engine/client_engine.hpp"
#include "engine/types.hpp"
#include "game.hpp"

Player::Player(Game &game) : game(game) {
    alive = true;
    canSpawn = true;

    physicsId = game.engine.physics->createPlayer(glm::vec3(1.0f, 2.0f, 1.0f));
    //renderId = game.engine.render->create("data/models/tank/tank.glb");
}

Player::~Player() {
    game.engine.physics->destroy(physicsId);
    //game.engine.render->destroy(renderId);
}

void Player::update() {
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

    glm::vec3 position = game.engine.physics->getPosition(physicsId);
    glm::quat rotation = game.engine.physics->getRotation(physicsId);

    this->location.position = position;
    this->location.rotation = rotation;

    //game.engine.render->setPosition(renderId, position);
    //game.engine.render->setRotation(renderId, rotation);

    game.engine.render->setCameraPosition(position + glm::vec3(0.0f, 0.0f, 0.0f));
    game.engine.render->setCameraRotation(rotation);
}