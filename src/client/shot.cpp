#include "shot.hpp"
#include "game.hpp"

Shot::Shot(Game &game, shot_id id, client_id ownerId, glm::vec3 position, glm::vec3 velocity) : game(game) {
    this->id = id;
    this->ownerId = ownerId;
    this->position = position;
    this->velocity = velocity;

    renderId = game.engine.render->create("data/models/shot.glb");
    game.engine.render->setPosition(renderId, position);
    game.engine.render->setScale(renderId, glm::vec3(0.6f));
    game.engine.render->setTransparency(renderId, true);

    ClientMsg_CreateShot createShotMsg;
    createShotMsg.localShotId = id;
    createShotMsg.position = position;
    createShotMsg.velocity = velocity;
    game.engine.network->send<ClientMsg_CreateShot>(createShotMsg);
}

Shot::~Shot() {
    game.engine.render->destroy(renderId);
}

void Shot::update(TimeUtils::duration deltaTime) {
    position += velocity * deltaTime;

    game.engine.render->setPosition(renderId, position);
}

