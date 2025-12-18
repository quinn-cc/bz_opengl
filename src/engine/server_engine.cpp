#include "engine/server_engine.hpp"
#include "engine/types.hpp"
#include <functional>

ServerEngine::ServerEngine(
    uint16_t serverPort,
    std::function<void(client_id)> connectionCallback,
    std::function<void(client_id)> disconnectionCallback
) {
    network = new ServerNetwork(
        serverPort,
        connectionCallback,
        disconnectionCallback
    );
    physics = new Physics();
}

ServerEngine::~ServerEngine() {
    delete network;
    delete physics;
}

void ServerEngine::update(TimeUtils::duration deltaTime) {
    network->update();
    physics->update(deltaTime);
}