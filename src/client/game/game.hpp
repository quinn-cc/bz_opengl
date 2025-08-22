#pragma once
#include <vector>
#include <algorithm>
#include "player.hpp"
#include "client.hpp"
#include "shot.hpp"

class Game {
public:
    Game() = default;
    
    std::vector<Shot *> shots;
    std::vector<Client *> clients;
    Player player;

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

    Client *GetClient(client_id clientId) {
        for (auto *client : clients) {
            if (client->IsClient(clientId)) {
                return client;
            }
        }
        return nullptr;
    }

    Shot *GetShotByLocalId(shot_id localId) {
        for (auto *shot : shots) {
            if (shot->GetId() == localId && shot->GetOwnerId() == 0) {
                return shot;
            }
        }
        return nullptr;
    }

    Shot *GetShotByGlobalId(shot_id globalId) {
        for (auto *shot : shots) {
            if (shot->GetId() == globalId && shot->GetOwnerId() != 0) {
                return shot;
            }
        }
        return nullptr;
    }
};