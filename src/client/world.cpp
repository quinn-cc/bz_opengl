#include "world.hpp"
#include "game.hpp"
#include "spdlog/spdlog.h"

World::World(Game &game) : game(game) {
    renderId = game.engine.render->create("data/world2.glb");
    physicsId = game.engine.physics->create("data/world2.glb", 0.0f);
}

World::~World() {
    game.engine.render->destroy(renderId);
    game.engine.physics->destroy(physicsId);
}

bool World::isInitialized() const {
    return initialized;
}

void World::update() {
    if (auto *initMsg = game.engine.network->peekMessage<ServerMsg_Init>()) {
        spdlog::trace("World::update: Received init message from server");
        // Load settings from init message
        defaultPlayerParams = initMsg->defaultPlayerParams;

        spdlog::info("World::update: World initialized from server");
        initialized = true;
        return;
    }
}