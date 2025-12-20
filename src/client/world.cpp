#include "world.hpp"
#include "game.hpp"

World::World(Game &game) : game(game) {
    settings.playerSpeed = 8.0f;
    settings.playerTurnSpeed = 2.0f;
    settings.gravity = -9.8f;
    settings.playerJumpSpeed = 5.0f;

    renderId = game.engine.render->create("data/world2.glb");
    physicsId = game.engine.physics->create("data/world2.glb", 0.0f);

    game.engine.physics->setGravity(settings.gravity);
}

World::~World() {
    game.engine.render->destroy(renderId);
    game.engine.physics->destroy(physicsId);
}

const WorldSettings &World::getSettings() const {
    return settings;
}