#include "player.hpp"
#include "engine/client_engine.hpp"
#include "engine/types.hpp"
#include "game.hpp"
#include <string>
#include "spdlog/spdlog.h"
#include "shot.hpp"

Player::Player(Game &game, client_id id, PlayerParameters params, const std::string name) : game(game) {
    this->clientId = id;
    this->state.params = params;
    state.name = name;
    state.alive = false;
    grounded = false;
    lastJumpTime = TimeUtils::GetCurrentTime();
    jumpCooldown = TimeUtils::getDuration(0.1f);

    physics = game.engine.physics->createPlayer(glm::vec3(1.0f, 2.0f, 1.0f));

    jumpAudioId = game.engine.audio->create(game.world->getAssetPath("playerJumpSound"), 5);
    dieAudioId = game.engine.audio->create(game.world->getAssetPath("playerDieSound"), 1);
    spawnAudioId = game.engine.audio->create(game.world->getAssetPath("playerSpawnSound"), 1);
    landAudioId = game.engine.audio->create(game.world->getAssetPath("playerLandSound"), 1);

    ClientMsg_Init initMsg;
    initMsg.name = name;
    game.engine.network->send<ClientMsg_Init>(initMsg);
}

Player::~Player() {
    physics.destroy();
}

float Player::getParameter(const std::string &paramName) const {
    auto it = state.params.find(paramName);
    if (it != state.params.end()) {
        return it->second;
    } else {
        spdlog::warn("Player::getParameter: Parameter '{}' not found, returning 0.0", paramName);
        return 0.0f;
    }
}

glm::vec3 Player::getForwardVector() const {
    return physics.getForwardVector();
}

void Player::earlyUpdate() {
    // Listen for incoming world setting changes
    if (auto *paramMsg = game.engine.network->peekMessage<ServerMsg_PlayerParameters>(
        [this](const ServerMsg_PlayerParameters &msg) {
            return msg.clientId == this->clientId;
        }))
    {
        for (auto& [key, value] : paramMsg->params) {
            this->state.params[key] = value;
        }
        game.engine.physics->setGravity(getParameter("gravity"));
    }

    if (state.alive) {
        game.engine.gui->displayDeathScreen(false);

        bool wasGrounded = grounded;
        grounded = physics.isGrounded(glm::vec3(1.0f, 2.0f, 1.0f));
        
        if (grounded) {
            glm::vec2 movement(0.0f);
            if (game.getFocusState() == FOCUS_STATE_GAME)
                movement = game.engine.input->getInputState().movement;
            glm::vec3 movementVector = physics.getForwardVector();
            movementVector *= movement.y * getParameter("speed");
            movementVector.y = physics.getVelocity().y;

            physics.setVelocity(movementVector);

            physics.setAngularVelocity(glm::vec3(
                0.0f,
                -movement.x * getParameter("turnSpeed"),
                0.0f
            ));

            if (game.getFocusState() == FOCUS_STATE_GAME) {
                if (grounded && game.engine.input->getInputState().jump && TimeUtils::GetElapsedTime(lastJumpTime, TimeUtils::GetCurrentTime()) >= jumpCooldown) {
                    glm::vec3 velocity = physics.getVelocity();
                    velocity.y = getParameter("jumpSpeed");
                    physics.setVelocity(velocity);
                    lastJumpTime = TimeUtils::GetCurrentTime();
                    grounded = false;
                    game.engine.audio->play(jumpAudioId, state.position);
                }
            }

            if (wasGrounded == false) {
                game.engine.audio->play(landAudioId, state.position);
            }
        }

        if (game.getFocusState() == FOCUS_STATE_GAME) {
            if (game.engine.input->getInputState().fire) {
                glm::vec3 shotPosition = state.position + getForwardVector() * 2.0f;
                glm::vec3 shotVelocity = getForwardVector() * getParameter("shotSpeed") + getVelocity();

                Shot *shot = new Shot(game, shotPosition, shotVelocity);
                game.addShot(shot);
            }
        }

        // Check if received death message
        if (auto *msg = game.engine.network->peekMessage<ServerMsg_PlayerDeath>(
            [this](const ServerMsg_PlayerDeath &msg) { return msg.clientId == this->clientId; }
        )) {
            game.engine.audio->play(dieAudioId, state.position);
            state.alive = false;
        }
    } else {
        game.engine.gui->displayDeathScreen(true);

        // Spawn key
        if (game.engine.input->getInputState().spawn) {
            ClientMsg_RequestPlayerSpawn spawnMsg;
            game.engine.network->send<ClientMsg_RequestPlayerSpawn>(spawnMsg);
        }
    }
}

void Player::lateUpdate() {
    if (!state.alive) {
        // If spawn command has been recieved
        if (auto *msg = game.engine.network->peekMessage<ServerMsg_PlayerSpawn>(
            [this](const ServerMsg_PlayerSpawn &msg) { return msg.clientId == this->clientId; }
        )) {
            game.engine.audio->play(spawnAudioId, msg->position);
            state.alive = true;
            physics.setPosition(msg->position);
            physics.setRotation(msg->rotation);
            physics.setVelocity(glm::vec3(0.0f));
            physics.setAngularVelocity(glm::vec3(0.0f));
        }
    }

    state.position = physics.getPosition();
    state.rotation = physics.getRotation();
    state.velocity = physics.getVelocity();
    game.engine.render->setCameraPosition(state.position);
    game.engine.render->setCameraRotation(state.rotation);

    if (glm::distance(lastPosition, state.position) > POSITION_UPDATE_THRESHOLD ||
        angleBetween(lastRotation, state.rotation) > ROTATION_UPDATE_THRESHOLD) {
        ClientMsg_PlayerLocation locMsg;
        locMsg.position = state.position;
        locMsg.rotation = state.rotation;
        game.engine.network->send<ClientMsg_PlayerLocation>(locMsg);
        lastPosition = state.position;
        lastRotation = state.rotation;
    }

    game.engine.audio->setListenerPosition(state.position);
    game.engine.audio->setListenerRotation(state.rotation);
}