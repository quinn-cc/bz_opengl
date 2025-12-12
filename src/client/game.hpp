#pragma once
#include "engine/engine.hpp"
#include "client.hpp"
#include "player.hpp"
#include "world.hpp"
#include "shot.hpp"

class Game {
public:
    Engine &engine;

    Player *player;
    World *world;
    std::vector<Client *> clients;
    std::vector<Shot *> shots;

    Game(Engine &engine);
    ~Game();

    void update(TimeUtils::duration deltaTime);
};