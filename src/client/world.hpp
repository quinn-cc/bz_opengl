#pragma once
#include <string>
#include "engine/types.hpp"

class Game;

struct WorldSettings {
    float playerSpeed;
    float playerTurnSpeed;
    float gravity;
};

class World {
private:
    Game &game;
    render_id renderId;
    WorldSettings settings;

public:
    World(Game &game);
    ~World();

    void load(std::string worldPath);
    const WorldSettings &getSettings() const;
};