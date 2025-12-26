#pragma once
#include "engine/types.hpp"

class Game;

class Shot {
private:
    Game &game;
    shot_id id;
    client_id ownerId;
    glm::vec3 position;
    glm::vec3 velocity;

    render_id renderId;

public:
    Shot(Game &game, shot_id id, client_id ownerId, glm::vec3 position, glm::vec3 velocity);
    ~Shot();

    void update(TimeUtils::duration deltaTime);
};