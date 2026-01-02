#include "client.hpp"
#include "engine/types.hpp"
#include "game.hpp"
#include "spdlog/spdlog.h"

Client::Client(Game &game, client_id id) : game(game), id(id) {
    initialized = false;
    alive = false;

    renderId = game.engine.render->create("data/models/tank/tank.glb");

    if (auto msg = game.engine.network->peekMessage<ServerMsg_PlayerJoin>(
        [this](const ServerMsg_PlayerJoin &msg) {
            return msg.clientId == this->id;
        }
    )) {
        location.position = msg->position;
        location.rotation = msg->rotation;
        //velocity = msg->location.velocity;
        name = std::string(msg->name);
        alive = msg->alive;
        spdlog::trace("Client::Client: Initialized location for client id {}", id);
    }

    game.engine.render->setPosition(renderId, location.position + glm::vec3(0.0f, -0.9f, 0.0f));
    game.engine.render->setRotation(renderId, location.rotation * glm::angleAxis(glm::pi<float>(), glm::vec3(0, 1, 0)));
    game.engine.render->setScale(renderId, glm::vec3(0.5f, 0.5f, 0.5f));
    game.engine.render->setVisible(renderId, alive);
}

Client::~Client() {
    game.engine.render->destroy(renderId);
}

void Client::update() {
    if (auto msg = game.engine.network->peekMessage<ServerMsg_PlayerLocation>(
        [this](const ServerMsg_PlayerLocation &msg) {
            return msg.clientId == this->id;
        }
    )) {
        location = msg->location;
        spdlog::trace("Client::update: Updated location for client id {}", id);
    }

    if (alive) {
        game.engine.render->setPosition(renderId, location.position + glm::vec3(0.0f, -0.8f, 0.0f));
        game.engine.render->setRotation(renderId, location.rotation * glm::angleAxis(glm::pi<float>(), glm::vec3(0, 1, 0)));

        // If receiving a death message
        if (auto msg = game.engine.network->peekMessage<ServerMsg_PlayerDeath>(
            [this](const ServerMsg_PlayerDeath &msg) {
                return msg.clientId == this->id;
            }
        )) {
            alive = false;
            game.engine.render->setVisible(renderId, false);
            spdlog::trace("Client::update: Client id {} has died", id);
        }
    } else {
        // If receiving a spawn message
        if (auto msg = game.engine.network->peekMessage<ServerMsg_PlayerSpawn>(
            [this](const ServerMsg_PlayerSpawn &msg) {
                return msg.clientId == this->id;
            }
        )) {
            location = msg->location;
            alive = true;
            game.engine.render->setPosition(renderId, location.position + glm::vec3(0.0f, -0.8f, 0.0f));
            game.engine.render->setRotation(renderId, location.rotation * glm::angleAxis(glm::pi<float>(), glm::vec3(0, 1, 0)));
            game.engine.render->setVisible(renderId, true);
            spdlog::trace("Client::update: Client id {} has spawned", id);
        }
    }
}

bool Client::isEqual(client_id otherId) {
    return this->id == otherId;
}

