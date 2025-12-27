#include "client.hpp"
#include "spdlog/spdlog.h"
#include "game.hpp"

Client::Client(Game &game, client_id id, std::string ip) : game(game), id(id), ip(ip) {
    initialized = false;
    alive = false;
}

Client::~Client() {
    ServerMsg_Disconnection serverDisconnMsg;
    serverDisconnMsg.clientId = id;
    game.engine.network->sendExcept<ServerMsg_Disconnection>(id, serverDisconnMsg);
}

bool Client::isEqual(client_id cid) const {
    return cid == id;
}

std::string Client::getName() const {
    return name;
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
            name = std::string(initMsg->name);
            initialized = true;

            // Notify all other clients about this new connection
            ServerMsg_Connection connMsg;
            connMsg.clientId = id;
            connMsg.alive = alive;
            strcpy(connMsg.name, name.c_str());
            connMsg.location = this->location;
            game.engine.network->sendExcept<ServerMsg_Connection>(id, connMsg);
        }
    }

    if (initialized) {
        // If this client sent a location update, update it and broadcast to all other clients
        if (auto *locMsg = game.engine.network->peekMessage<ClientMsg_Location>([this](const ClientMsg_Location &msg) {
            return msg.clientId == id;
        })) { 
            this->location = locMsg->location;

            ServerMsg_Location updateMsg;
            updateMsg.clientId = id;
            updateMsg.location = this->location;
            game.engine.network->sendExcept<ServerMsg_Location>(id, updateMsg);
        }

        // If another client joins then send them the connection message of this client
        if (auto *msg = game.engine.network->peekMessage<ClientMsg_Connection>([this](const ClientMsg_Connection &msg) {
            return msg.clientId != id;
        })) {
            ServerMsg_Connection connMsg;
            connMsg.clientId = id;
            connMsg.alive = alive;
            strcpy(connMsg.name, name.c_str());
            connMsg.location = this->location;
            game.engine.network->send<ServerMsg_Connection>(msg->clientId, connMsg);
        }
    }

    if (auto *spawnMsg = game.engine.network->peekMessage<ClientMsg_RequestSpawn>([this](const ClientMsg_RequestSpawn &msg) {
        return msg.clientId == id;
    })) {
        if (alive) {
            spdlog::warn("Client::update: Client id {} requested spawn while already alive", id);
        } else {
            Location spawnLocation = game.world->getSpawnLocation();
            this->location = spawnLocation;

            // Send spawn command to this client
            ServerMsg_Spawn spawnRespMsg;
            spawnRespMsg.clientId = 0;
            spawnRespMsg.location = this->location;
            game.engine.network->send<ServerMsg_Spawn>(id, spawnRespMsg);

            // Send to all other clients
            ServerMsg_Spawn spawnBroadcastMsg;
            spawnBroadcastMsg.clientId = id;
            spawnBroadcastMsg.location = this->location;
            game.engine.network->sendExcept<ServerMsg_Spawn>(id, spawnBroadcastMsg);

            alive = true;
        }
    }
}

void Client::die() {
    alive = false;

    // Tell this client to die
    ServerMsg_Death deathRespMsg;
    deathRespMsg.clientId = 0;
    game.engine.network->send<ServerMsg_Death>(id, deathRespMsg);

    // Broadcast to everyone else
    ServerMsg_Death deathMsg;
    deathMsg.clientId = id;
    game.engine.network->sendExcept<ServerMsg_Death>(id, deathMsg);
}