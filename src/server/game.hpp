#pragma once
#include "engine/types.hpp"
#include "engine/server_engine.hpp"
#include "client.hpp"
#include "shot.hpp"
#include <vector>

class Game {
private:
    std::vector<Client *> clients;
    void addClient(Client *client);
    void removeClient(client_id id);
    Client *getClient(client_id id);

    std::vector<Shot *> shots;

public:
    ServerEngine &engine;
    const std::vector<Client *> &getClients() const { return clients; }

    Game(class ServerEngine &engine);
    ~Game() = default;

    void update(TimeUtils::duration deltaTime);
};