#pragma once
#include <vector>
#include "engine/types.hpp"
#include "engine/client_engine.hpp"
#include "player.hpp"
#include "world.hpp"
#include "shot.hpp"
#include "console.hpp"
#include "client.hpp"

enum FOCUS_STATE {
    FOCUS_STATE_GAME,
    FOCUS_STATE_CONSOLE
};

class Game {
private:
    FOCUS_STATE focusState;

public:
    ClientEngine &engine;

    Player *player;
    World *world;
    Console *console;

    std::vector<Client *> clients;
    std::vector<Shot *> shots;
    shot_id getNextLocalShotId() {
        static shot_id nextId = 1;
        return nextId++;
    }

    FOCUS_STATE getFocusState() const { return focusState; }

    Game(ClientEngine &engine, std::string playerName);
    ~Game();

    void update(TimeUtils::duration deltaTime);
};