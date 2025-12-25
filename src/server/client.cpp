#include "client.hpp"
#include "spdlog/spdlog.h"
#include "game.hpp"

Client::Client(Game &game, client_id id, std::string ip) : game(game), id(id), ip(ip) {
    initialized = false;
}

bool Client::isClient(client_id cid) const {
    return cid == id;
}

void Client::update() {
    ClientMsg_Init *initMsg = game.engine.network->peekMessage<ClientMsg_Init>([this](const ClientMsg_Init &msg) {
        return msg.clientId == id;
    });

    if (initMsg) {
        if (initialized) {
            spdlog::warn("Client::update: Client id {} sent duplicate initialization message", id);
            game.engine.network->popMessage(initMsg);
            return;
        } else {
            spdlog::info("Client::update: Client id {} initialized with name {}", id, initMsg->name);
            name = std::string(initMsg->name);
            game.engine.network->popMessage(initMsg);
            initialized = true;

            ServerMsg_Connection connMsg;
            connMsg.clientId = id;
            snprintf(connMsg.name, sizeof(connMsg.name), "%s", name.c_str());
            game.engine.network->sendExcept<ServerMsg_Connection>(id, connMsg);
        }
    }

    if (initialized) {
        // Update location        
        ClientMsg_Location *locMsg = game.engine.network->peekMessage<ClientMsg_Location>([this](const ClientMsg_Location &msg) {
            return msg.clientId == id;
        });

        if (locMsg) { 
            this->location = locMsg->location;
            game.engine.network->popMessage(locMsg);

            ServerMsg_Location updateMsg;
            updateMsg.clientId = id;
            updateMsg.location = this->location;
            game.engine.network->sendExcept<ServerMsg_Location>(id, updateMsg);
        }
    } else {
        ClientMsg *msg = game.engine.network->peekMessage<ClientMsg>([this](const ClientMsg &msg) {
            return msg.clientId == id;
        });

        if (msg) {
            spdlog::warn("Client::update: Client id {} sent message before initialization", id);
        }
    }
}