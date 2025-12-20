#include "engine/server_engine.hpp"
#include "engine/types.hpp"
#include <functional>

ServerEngine::ServerEngine(uint16_t serverPort) {
    network = new ServerNetwork(serverPort);
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