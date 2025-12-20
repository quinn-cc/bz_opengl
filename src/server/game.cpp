#include "game.hpp"
#include "spdlog/spdlog.h"

Game::Game(ServerEngine &engine) : engine(engine) {
    
}

void Game::update(TimeUtils::duration deltaTime) {
    ClientMsg_Connection *connMsg = engine.network->peekMessage<ClientMsg_Connection>();

    if (connMsg) {
        spdlog::info("Game::update: New client connected with id {} at ip {}", connMsg->clientId, connMsg->ip);
        clients.push_back(Client(connMsg->clientId, std::string(connMsg->ip)));
        engine.network->popMessage(connMsg);
    }

    ClientMsg_Disconnection *disconnMsg = engine.network->peekMessage<ClientMsg_Disconnection>();

    if (disconnMsg) {
        spdlog::info("Game::update: Client with id {} disconnected", disconnMsg->clientId);
        clients.erase(
            std::remove_if(
                clients.begin(),
                clients.end(),
                [disconnMsg](const Client &c) { return c.isClient(disconnMsg->clientId); }
            ),
            clients.end()
        );
        engine.network->popMessage(disconnMsg);
    }
}