#include "world.hpp"
#include "engine/engine.hpp"

World::World(Engine &engine) : engine(engine) {
    playerSpeed = 10.0f;
    playerTurnSpeed = 5.0f;

    renderId = engine.render->create("data/world.glb");
}

World::~World() {
    engine.render->destroy(renderId);
}
