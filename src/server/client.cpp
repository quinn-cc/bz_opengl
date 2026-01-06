#include "client.hpp"
#include "spdlog/spdlog.h"
#include "game.hpp"

Client::Client(Game &game, client_id id, std::string ip) : game(game), id(id), ip(ip) { 
    initialized = false;

    state.name = "empty";
    state.position = glm::vec3(0.0f, 0.0f, 0.0f);
    state.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    state.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    state.alive = false;
}

Client::~Client() {
    ServerMsg_PlayerLeave serverDisconnMsg;
    serverDisconnMsg.clientId = id;
    game.engine.network->sendExcept<ServerMsg_PlayerLeave>(id, &serverDisconnMsg);
}

bool Client::isEqual(client_id cid) const {
    return cid == id;
}

std::string Client::getName() const {
    return state.name;
}

void Client::update() {
    if (auto *initMsg = game.engine.network->peekMessage<ClientMsg_Init>([this](const ClientMsg_Init &msg) {
        return msg.clientId == id;
    })) {
        if (initialized) {
            spdlog::warn("Client::update: Client id {} sent duplicate initialization message", id);
            return;
        } else {
            spdlog::info("Client::update: Client id {} initialized with name {}", id, initMsg->name);
            state.name = initMsg->name;
            initialized = true;

            // Notify all other clients about this new connection
            ServerMsg_PlayerJoin connMsg;
            connMsg.clientId = id;
            connMsg.state = state;
            game.engine.network->sendExcept<ServerMsg_PlayerJoin>(id, &connMsg);
        }
    }

    if (initialized) {
        // If this client sent a location update, update it and broadcast to all other clients
        if (auto *locMsg = game.engine.network->peekMessage<ClientMsg_PlayerLocation>([this](const ClientMsg_PlayerLocation &msg) {
            return msg.clientId == id;
        })) { 
            state.position = locMsg->position;
            state.rotation = locMsg->rotation;

            ServerMsg_PlayerLocation updateMsg;
            updateMsg.clientId = id;
            updateMsg.position = state.position;
            updateMsg.rotation = state.rotation;
            game.engine.network->sendExcept<ServerMsg_PlayerLocation>(id, &updateMsg);
        }

        // If another client joins then send them the connection message of this client
        if (auto *msg = game.engine.network->peekMessage<ClientMsg_PlayerJoin>([this](const ClientMsg_PlayerJoin &msg) {
            return msg.clientId != id;
        })) {
            ServerMsg_PlayerJoin connMsg;
            connMsg.clientId = id;
            connMsg.state = state;
            game.engine.network->send<ServerMsg_PlayerJoin>(msg->clientId, &connMsg);
        }
    }

    if (auto *spawnMsg = game.engine.network->peekMessage<ClientMsg_RequestPlayerSpawn>([this](const ClientMsg_RequestPlayerSpawn &msg) {
        return msg.clientId == id;
    })) {
        if (state.alive) {
            spdlog::warn("Client::update: Client id {} requested spawn while already alive", id);
        } else {
            Location spawnLocation = game.world->getSpawnLocation();
            state.position = spawnLocation.position;
            state.rotation = spawnLocation.rotation;

            // Send spawn command to this client
            ServerMsg_PlayerSpawn spawnRespMsg;
            spawnRespMsg.clientId = id;
            spawnRespMsg.position = state.position;
            spawnRespMsg.rotation = state.rotation;
            game.engine.network->sendAll<ServerMsg_PlayerSpawn>(&spawnRespMsg);

            state.alive = true;
        }
    }
}

void Client::die() {
    if (state.alive) {
        state.alive = false;

        // Broadcast to everyone else
        ServerMsg_PlayerDeath deathMsg;
        deathMsg.clientId = id;
        game.engine.network->sendAll<ServerMsg_PlayerDeath>(&deathMsg);
    }
}