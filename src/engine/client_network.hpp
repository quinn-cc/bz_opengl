#pragma once
#include "engine/types.hpp"
#include <string>
#include <cstdint>
#include <enet.h>

class ClientNetwork {
private:
    ENetHost* client = nullptr;
    ENetPeer* server = nullptr;

public:
    ClientNetwork() = default;
    ~ClientNetwork() = default;

    void connect(const std::string &address, uint16_t port);
    void update();

    std::optional<ServerMsg> poll(ServerMsg_Type type);
    void send(const ClientMsg &msg, bool flush = false);
};