#pragma once
#include "enet.h"
#include <string>
#include <optional>
#include "netmsg.hpp"

class Shot;

class Networker {
private:
    ENetHost* client;
    ENetPeer* server;

    // void MsgRecv(ENetEvent event);
    
public:

    int Init();
    void Connect(std::string addr);
    void Update(std::function<void(ServerMsg *)> callback);

    template<typename T> void Send(const T msg, std::optional<bool> flush = false) {
        ENetPacketFlag flag = ENET_PACKET_FLAG_RELIABLE;

        if constexpr (std::is_same_v<T, ClientMsg_Location>) {
            flag = ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
        }

        ENetPacket* packet = enet_packet_create(reinterpret_cast<const void*>(&msg), sizeof(T), flag);
        enet_peer_send(server, 0, packet);

        if (flush.has_value() && flush.value()) {
            enet_host_flush(client);
        }
    }

    void Close();
};