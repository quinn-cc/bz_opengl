#include "game.hpp"
#include "spdlog/spdlog.h"

void Game::addClient(Client *client) {
    clients.push_back(client);
}

void Game::removeClient(client_id id) {
    clients.erase(
        std::remove_if(
            clients.begin(),
            clients.end(),
            [id](const Client *c) {
                if (c->isEqual(id)) {
                    delete c;
                    return true;
                } else {
                    return false;
                }
            }
        ),
        clients.end()
    );
}

Client *Game::getClient(client_id id) {
    for (Client *client : clients) {
        if (client->isEqual(id)) {
            return client;
        }
    }
    return nullptr;
}

Game::Game(ServerEngine &engine) : engine(engine) {
    
}

void Game::update(TimeUtils::duration deltaTime) {
    for (Client *client : clients) {
        client->update();
    }

    if (ClientMsg_Connection *connMsg = engine.network->peekMessage<ClientMsg_Connection>()) {
        spdlog::info("Game::update: New client connected with id {} at ip {}", connMsg->clientId, connMsg->ip);
        Client *newClient = new Client(*this, connMsg->clientId, std::string(connMsg->ip));
        addClient(newClient);
    }

    if (ClientMsg_Disconnection *disconnMsg = engine.network->peekMessage<ClientMsg_Disconnection>()) {
        spdlog::info("Game::update: Client with id {} disconnected", disconnMsg->clientId);
        removeClient(disconnMsg->clientId);
    }

    // Listen for incoming chat
    if (auto *chatMsg = engine.network->peekMessage<ClientMsg_Chat>()) {
        spdlog::info("Game::update: Received chat message from client id {}: {}", chatMsg->clientId, chatMsg->text);

        ServerMsg_Chat serverChatMsg;
        strcpy(serverChatMsg.name, getClient(chatMsg->clientId)->getName().c_str());
        strcpy(serverChatMsg.text, chatMsg->text);
        engine.network->sendExcept<ServerMsg_Chat>(chatMsg->clientId, serverChatMsg);
    }

    // Listen for incoming shots
    if (auto *shotMsg = engine.network->peekMessage<ClientMsg_CreateShot>()) {
        Shot *newShot = new Shot(
            *this,
            shotMsg->clientId,
            shotMsg->localShotId,
            shotMsg->position,
            shotMsg->velocity
        );
        shots.push_back(newShot);
    }
}