#include "world.hpp"
#include "engine/engine.hpp"
#include "game.hpp"

World::World(Game &game) : game(game) {
    settings.playerSpeed = 10.0f;
    settings.playerTurnSpeed = 5.0f;
    settings.gravity = -9.8f;

    renderId = game.engine.render->create("data/world.glb");
}

World::~World() {
    game.engine.render->destroy(renderId);
}

const WorldSettings &World::getSettings() const {
    return settings;
}