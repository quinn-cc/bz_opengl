#pragma once
#include "engine/types.hpp"
#include "engine/server_engine.hpp"
#include "client.hpp"
#include "shot.hpp"
#include "world.hpp"
#include "chat.hpp"
#include <vector>

class Game {
private:
    std::vector<Client *> clients;
    void addClient(Client *client);
    void removeClient(client_id id);

    std::vector<Shot *> shots;

public:
    ServerEngine &engine;
    World *world;
    Chat *chat;

    const std::vector<Client *> &getClients() const { return clients; }
    Client *getClient(client_id id);

    Game(class ServerEngine &engine);
    ~Game();

    void update(TimeUtils::duration deltaTime);
};