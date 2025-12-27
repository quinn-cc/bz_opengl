#pragma once
#include <string>
#include "engine/types.hpp"

class Game;

class Client {
private:
    Game &game;
    
    bool initialized;
    bool alive;
    std::string ip;
    std::string name;
    client_id id;
    Location location;

public:
    Client(Game &game, client_id id, std::string ip);
    ~Client();

    bool isEqual(client_id cid) const;
    std::string getName() const;
    void update();
    glm::vec3 getPosition() const { return location.position; }
    void die();
};