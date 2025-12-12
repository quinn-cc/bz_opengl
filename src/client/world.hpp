#pragma once
#include <string>

class Game;

class World {
private:
    Game &game;

    render_id renderId;

    float playerSpeed;
    float playerTurnSpeed;

public:
    World(Game &game);
    ~World() = default;

    void load(std::string worldPath);

    float getPlayerSpeed() const;
    float getPlayerTurnSpeed() const;
};