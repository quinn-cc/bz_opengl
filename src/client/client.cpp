#include "client.hpp"
#include "engine/types.hpp"
#include "game.hpp"
#include "spdlog/spdlog.h"

Client::Client(Game &game, client_id id) : game(game), id(id) {
    initialized = false;

    renderId = game.engine.render->create("data/models/tank/tank.glb");

    if (auto msg = game.engine.network->peekMessage<ServerMsg_Connection>(
        [this](const ServerMsg_Connection &msg) {
            return msg.clientId == this->id;
        }
    )) {
        location = msg->location;
        name = std::string(msg->name);
        alive = msg->alive;
        spdlog::trace("Client::Client: Initialized location for client id {}", id);
    }

    game.engine.render->setPosition(renderId, location.position + glm::vec3(0.0f, -0.9f, 0.0f));
    game.engine.render->setRotation(renderId, location.rotation * glm::angleAxis(glm::pi<float>(), glm::vec3(0, 1, 0)));
    game.engine.render->setScale(renderId, glm::vec3(0.5f, 0.5f, 0.5f));
}

Client::~Client() {
    game.engine.render->destroy(renderId);
}

void Client::update() {
    if (auto msg = game.engine.network->peekMessage<ServerMsg_Location>(
        [this](const ServerMsg_Location &msg) {
            return msg.clientId == this->id;
        }
    )) {
        location = msg->location;
        game.engine.network->popMessage(msg);
        spdlog::trace("Client::update: Updated location for client id {}", id);
    }

    game.engine.render->setPosition(renderId, location.position + glm::vec3(0.0f, -0.8f, 0.0f));
    game.engine.render->setRotation(renderId, location.rotation * glm::angleAxis(glm::pi<float>(), glm::vec3(0, 1, 0)));
}

bool Client::isEqual(client_id otherId) {
    return this->id == otherId;
}

