#pragma once
#include "engine/types.hpp"

class Game;

class Shot {
private:
    Game &game;
    client_id ownerId;
    shot_id localId;
    shot_id globalId;
    glm::vec3 position;
    glm::vec3 velocity;

    shot_id getNextGlobalShotId() {
        static shot_id nextId = 1;
        return nextId++;
    }

public:
    Shot(Game &game, client_id ownerId, shot_id localShotId, glm::vec3 position, glm::vec3 velocity);
    ~Shot();

    void update(TimeUtils::duration deltaTime);
};