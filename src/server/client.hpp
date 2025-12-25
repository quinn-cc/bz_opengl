#pragma once
#include <string>
#include "engine/types.hpp"

class Game;

class Client {
private:
    Game &game;
    
    bool initialized;
    std::string ip;
    std::string name;
    client_id id;
    Location location;

public:
    Client(Game &game, client_id id, std::string ip);
    ~Client() = default;

    bool isClient(client_id cid) const;
    void update();
};