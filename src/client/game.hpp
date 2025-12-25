#pragma once
#include <vector>
#include "engine/types.hpp"
#include "engine/client_engine.hpp"
#include "player.hpp"
#include "world.hpp"
#include "shot.hpp"

class Game {
public:
    ClientEngine &engine;

    Player *player;
    World *world;
    std::vector<Client *> clients;
    std::vector<Shot *> shots;

    Game(ClientEngine &engine, std::string playerName);
    ~Game();

    void update(TimeUtils::duration deltaTime);
};