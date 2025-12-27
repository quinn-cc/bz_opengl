#include "shot.hpp"
#include "game.hpp"
#include "spdlog/spdlog.h"

Shot::Shot(Game &game, shot_id id, bool isGlobalId, glm::vec3 position, glm::vec3 velocity) : game(game) {
    this->id = id;
    this->isGlobalId = isGlobalId;
    this->position = position;
    this->velocity = velocity;

    renderId = game.engine.render->create("data/models/shot.glb");
    game.engine.render->setPosition(renderId, position);
    game.engine.render->setScale(renderId, glm::vec3(0.6f));
    game.engine.render->setTransparency(renderId, true);   
}

// Local id constructor
Shot::Shot(Game &game, glm::vec3 position, glm::vec3 velocity) : Shot(game, getNextLocalShotId(), false, position, velocity) {
    ClientMsg_CreateShot createShotMsg;
    createShotMsg.localShotId = id;
    createShotMsg.position = position;
    createShotMsg.velocity = velocity;
    game.engine.network->send<ClientMsg_CreateShot>(createShotMsg);
};

// Global id constructor
Shot::Shot(Game &game, shot_id globalId, glm::vec3 position, glm::vec3 velocity) : Shot(game, globalId, true, position, velocity) {};

Shot::~Shot() {
    game.engine.render->destroy(renderId);
}

void Shot::update(TimeUtils::duration deltaTime) {
    position += velocity * deltaTime;

    game.engine.render->setPosition(renderId, position);
}

