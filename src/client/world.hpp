#pragma once
#include <string>
#include "engine/types.hpp"

class Game;

struct WorldSettings {
    float playerSpeed;
    float playerTurnSpeed;
    float playerJumpSpeed;
    float gravity;
    float shotSpeed;
};

class World {
private:
    Game &game;
    render_id renderId;
    physics_id physicsId;
    WorldSettings settings;

public:
    World(Game &game);
    ~World();

    void load(std::string worldPath);
    const WorldSettings &getSettings() const;
};