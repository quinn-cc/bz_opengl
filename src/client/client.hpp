#pragma once
#include <cstdint>
#include "engine/types.hpp"

class Game;

class Client {
private:
    Game &game;
    bool initialized;
    client_id id;
    render_id renderId;

    Location location;
    bool alive;
    std::string name;

public:
    Client(Game &game, client_id id);
    ~Client();

    void update();
    bool isEqual(client_id otherId);
    std::string getName() const { return name; }
};