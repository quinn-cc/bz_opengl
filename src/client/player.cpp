#include "player.hpp"
#include "engine/client_engine.hpp"
#include "engine/types.hpp"
#include "game.hpp"
#include <string>
#include "spdlog/spdlog.h"

Player::Player(Game &game, const std::string name) : game(game) {
    this->name = name;
    alive = false;
    grounded = false;
    lastJumpTime = TimeUtils::GetCurrentTime();
    jumpCooldown = TimeUtils::getDuration(0.1f);

    physicsId = game.engine.physics->createPlayer(glm::vec3(1.0f, 2.0f, 1.0f));

    jumpAudioId = game.engine.audio->create("data/audio/jump.wav", 5);
    dieAudioId = game.engine.audio->create("data/audio/die.wav", 1);
    spawnAudioId = game.engine.audio->create("data/audio/spawn.wav", 1);
    landAudioId = game.engine.audio->create("data/audio/land.wav", 1);

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

void Player::earlyUpdate() {
    if (alive) {
        game.engine.gui->displayDeathScreen(false);

        bool wasGrounded = grounded;
        grounded = game.engine.physics->isGrounded(physicsId, glm::vec3(1.0f, 2.0f, 1.0f));       
        
        if (grounded) {
            glm::vec2 movement(0.0f);
            if (game.getFocusState() == FOCUS_STATE_GAME)
                movement = game.engine.input->getInputState().movement;
            float speed = game.world->getSetting("playerSpeed");
            float turnSpeed = game.world->getSetting("playerTurnSpeed");

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
                    velocity.y = game.world->getSetting("playerJumpSpeed");
                    game.engine.physics->setVelocity(physicsId, velocity);
                    lastJumpTime = TimeUtils::GetCurrentTime();
                    grounded = false;
                    game.engine.audio->play(jumpAudioId, location.position);
                }
            }

            if (wasGrounded == false) {
                game.engine.audio->play(landAudioId, location.position);
            }
        }

        // Check if received death message
        if (auto *msg = game.engine.network->peekMessage<ServerMsg_Death>(
            [this](const ServerMsg_Death &msg) { return msg.clientId == 0; }
        )) {
            game.engine.audio->play(dieAudioId, location.position);
            alive = false;
        }
    } else {
        game.engine.gui->displayDeathScreen(true);

        // Spawn key
        if (game.engine.input->getInputState().spawn) {
            ClientMsg_RequestSpawn spawnMsg;
            game.engine.network->send<ClientMsg_RequestSpawn>(spawnMsg);
        }
    }
}

void Player::lateUpdate() {
    if (!alive) {
        // If spawn command has been recieved
        if (auto *msg = game.engine.network->peekMessage<ServerMsg_Spawn>(
            [this](const ServerMsg_Spawn &msg) { return msg.clientId == 0; }
        )) {
            game.engine.audio->play(spawnAudioId, msg->location.position);
            alive = true;
            game.engine.physics->setPosition(physicsId, msg->location.position);
            game.engine.physics->setRotation(physicsId, msg->location.rotation);
            game.engine.physics->setVelocity(physicsId, glm::vec3(0.0f));  // Add this
            game.engine.physics->setAngularVelocity(physicsId, glm::vec3(0.0f));  // Add this too
        }
    }

    this->location.position = game.engine.physics->getPosition(physicsId);
    this->location.rotation = game.engine.physics->getRotation(physicsId);
    this->velocity = game.engine.physics->getVelocity(physicsId);
    game.engine.render->setCameraPosition(this->location.position);
    game.engine.render->setCameraRotation(this->location.rotation);

    if (glm::distance(this->lastLocation.position, this->location.position) > POSITION_UPDATE_THRESHOLD ||
        angleBetween(this->lastLocation.rotation, this->location.rotation) > ROTATION_UPDATE_THRESHOLD) {
        ClientMsg_Location locMsg;
        locMsg.location = this->location;
        game.engine.network->send<ClientMsg_Location>(locMsg);
        this->lastLocation = this->location;
    }

    game.engine.audio->setListenerPosition(this->location.position);
    game.engine.audio->setListenerRotation(this->location.rotation);
}