#pragma once
#include "engine/types.hpp"
#include "engine/server_engine.hpp"
#include "client.hpp"
#include <vector>

class Game {
public:
    ServerEngine &engine;

    std::vector<Client *> clients;

    Game(class ServerEngine &engine);
    ~Game() = default;

    void update(TimeUtils::duration deltaTime);
};