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

    struct MsgData {
        ENetPacket* packet;
        ServerMsg* msg;
    };

    std::vector<MsgData> receivedMessages;

    ClientNetwork();
    ~ClientNetwork();

    void update();

public:
    bool connect(const std::string &address, uint16_t port, int timeoutMs = 5000);

    template<typename T> T* peekMessage(std::function<bool(const T&)> predicate = [](const T&) { return true; }) const {
        static_assert(std::is_base_of_v<ServerMsg, T>, "T must be a subclass of ServerMsg");

        for (auto msgData : receivedMessages) {
            if (msgData.msg->type == T::Type) {
                auto* casted = static_cast<T*>(msgData.msg);
                if (predicate(*casted)) {
                    return casted;
                }
            }
        }

        return nullptr;
    };

    void popMessage(ServerMsg* msg);

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

    
};