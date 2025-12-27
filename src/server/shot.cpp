#include "shot.hpp"
#include "game.hpp"

Shot::Shot(Game &game, client_id ownerId, shot_id localShotId, glm::vec3 position, glm::vec3 velocity) : game(game) {
    this->ownerId = ownerId;
    this->localId = localShotId;
    this->position = position;
    this->velocity = velocity;

    this->globalId = getNextGlobalShotId();

    ServerMsg_CreateShot serverShotMsg;
    serverShotMsg.globalShotId = globalId;
    serverShotMsg.position = position;
    serverShotMsg.velocity = velocity;
    game.engine.network->sendExcept<ServerMsg_CreateShot>(ownerId, serverShotMsg);
}

Shot::~Shot() {
    // Local remove message to owner
    ServerMsg_RemoveShot localRemoveMsg;
    localRemoveMsg.isGlobalId = false;
    localRemoveMsg.shotId = localId;
    game.engine.network->send<ServerMsg_RemoveShot>(ownerId, localRemoveMsg);

    // Global remove message to everyone else
    ServerMsg_RemoveShot globalRemoveMsg;
    globalRemoveMsg.isGlobalId = true;
    globalRemoveMsg.shotId = globalId;
    game.engine.network->sendExcept<ServerMsg_RemoveShot>(ownerId, globalRemoveMsg);
}

void Shot::update(TimeUtils::duration deltaTime) {
    position += velocity * deltaTime;
}