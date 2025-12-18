#include "player.hpp"
#include "engine/client_engine.hpp"
#include "engine/types.hpp"
#include "game.hpp"

Player::Player(Game &game) : game(game) {
    alive = true;
    canSpawn = true;

    physics_id physicsId = game.engine.physics->createPlayer(glm::vec3(1.0f, 2.0f, 1.0f), 1.0f);
    render_id renderId = game.engine.render->create("data/models/tank/tank.glb");
}

Player::~Player() {
    game.engine.physics->destroy(physicsId);
    game.engine.render->destroy(renderId);
}

void Player::update() {
    glm::vec2 movement = game.engine.input->getInputState().movement;
    float speed = game.world->getSettings().playerSpeed;
    float turnSpeed = game.world->getSettings().playerTurnSpeed;

    game.engine.physics->setVelocity(physicsId, glm::vec3(
        movement.y * speed,
        0.0f,
        -movement.x * speed
    ));

    glm::vec3 position = game.engine.physics->getPosition(physicsId);
    glm::quat rotation = game.engine.physics->getRotation(physicsId);

    this->location.position = position;
    this->location.rotation = rotation;

    game.engine.render->setPosition(renderId, position);
    game.engine.render->setRotation(renderId, rotation);
}