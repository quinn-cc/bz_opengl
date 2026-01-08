#include "game.hpp"
#include "spdlog/spdlog.h"
#include <utility>

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

Client *Game::getClientByName(const std::string &name) {
    for (Client *client : clients) {
        if (client->isEqual(name)) {
            return client;
        }
    }
    return nullptr;
}

Game::Game(ServerEngine &engine,
           std::string worldName,
           nlohmann::json worldConfig,
           std::string worldDir,
           bool enableWorldZipping)
    : engine(engine) {
    world = new World(*this,
                      std::move(worldName),
                      std::move(worldConfig),
                      std::move(worldDir),
                      enableWorldZipping);
    chat = new Chat(*this);
}

Game::~Game() {
    for (Client *client : clients) {
        delete client;
    }
    clients.clear();

    for (Shot *shot : shots) {
        delete shot;
    }
    shots.clear();

    delete world;
    delete chat;
}

void Game::update(TimeUtils::duration deltaTime) {
    chat->update();

    for (Client *client : clients) {
        client->update();
    }

    if (ClientMsg_PlayerJoin *connMsg = engine.network->peekMessage<ClientMsg_PlayerJoin>()) {
        Client *newClient = new Client(*this, connMsg->clientId, connMsg->ip);
        addClient(newClient);
    }

    if (ClientMsg_PlayerLeave *disconnMsg = engine.network->peekMessage<ClientMsg_PlayerLeave>()) {
        spdlog::info("Game::update: Client with id {} disconnected", disconnMsg->clientId);
        removeClient(disconnMsg->clientId);
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

    for (Shot *shot : shots) {
        shot->update(deltaTime);

        if (shot->isExpired()) {
            shots.erase(
                std::remove(shots.begin(), shots.end(), shot),
                shots.end()
            );
            delete shot;
            continue;
        }

        for (Client *client : clients) {
            if (shot->hits(client)) {
                client->die();

                shots.erase(
                    std::remove(shots.begin(), shots.end(), shot),
                    shots.end()
                );
                delete shot;
                break;
            }
        }
    }

    world->update();
}