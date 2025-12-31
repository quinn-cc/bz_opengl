#pragma once
#include <vector>
#include "client.hpp"
#include "shot.hpp"
#include "runtime.hpp"

class Game {
public:
    std::vector<Client *> clients;
    std::vector<Shot *> shots;
    Runtime runtime;

    Client *GetClient(client_id clientId) {
        for (auto *client : clients) {
            if (client->id == clientId) {
                return client;
            }
        }
        return nullptr;
    }

    void AddShot(Shot *shot) {
        shots.push_back(shot);
    }

    void AddClient(Client *client) {
        clients.push_back(client);
    }

    void RemoveShot(Shot *shot) {
        shots.erase(std::remove(shots.begin(), shots.end(), shot), shots.end());
    }

    void RemoveClient(Client *client) {
        clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
    }
};