#pragma once
#include "engine/types.hpp"
#include <string>
#include <cstdint>
#include <enet.h>
#include <optional>
#include <map>
#include <functional>

class ServerNetwork {
private:
    ENetHost *server = nullptr;
    std::map<client_id, ENetPeer *> clients;
    void update();

    friend class Engine;

public:
    ServerNetwork(
        uint16_t port,
        std::function<void(client_id)> connectionCallback,
        std::function<void(client_id)> disconnectionCallback,
        std::optional<int> maxClients = 50,
        std::optional<int> numChannels = 2
    );
    ~ServerNetwork() = default;

    ClientMsg poll(ClientMsg_Type type);
    template<typename T> void send(
        client_id client, 
        const T msg, 
        std::optional<bool> flush = false);
    template<typename T> void sendExcept(
        client_id excludeClient,
        const T msg,
        std::optional<bool> flush = false
    );

    std::vector<client_id> getClients();
};