#pragma once
#include "engine/types.hpp"
#include <string>
#include <cstdint>
#include <enet.h>

class ClientNetwork {
    friend class ClientEngine;

private:
    ENetHost* client = nullptr;
    ENetPeer* server = nullptr;

    std::vector<ServerMsg*> receivedMessages;

    ClientNetwork();
    ~ClientNetwork();

    void update();

public:
    void connect(const std::string &address, uint16_t port);

    template<typename T> std::optional<T*> peek() const {
        static_assert(std::is_base_of_v<ServerMsg, T>, "T must be a subclass of ServerMsg");

        for (auto msg : receivedMessages) {
            if (msg->type == T::Type) {
                return reinterpret_cast<T*>(msg);
            }
        }

        return std::nullopt;
    };

    template<typename T> void send(const T &msg, bool flush = false) {
        static_assert(std::is_base_of_v<ClientMsg, T>, "T must be a subclass of ClientMsg");

        ENetPacketFlag flag = ENET_PACKET_FLAG_RELIABLE;

        if constexpr (std::is_same_v<T, ClientMsg_Location>) {
            flag = ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
        }

        ENetPacket* packet = enet_packet_create(reinterpret_cast<const void*>(&msg), sizeof(T), flag);
        enet_peer_send(server, 0, packet);

        if (flush) {
            enet_host_flush(client);
        }
    };

    void pop(ServerMsg* msg);
};