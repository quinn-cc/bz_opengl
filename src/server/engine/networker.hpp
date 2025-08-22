#pragma once
#include <map>
#include <enet.h>
#include "netmsg.hpp"
#include "shot.hpp"
#include "client.hpp"

#define LISTEN_PORT 1234
#define MAX_CLIENTS 50
#define NUM_CHANNELS 2


class Networker {
private:
    std::map<Client *, ENetPeer *> peers;
    ENetHost *server;

public:
    int Init();
    void Update(
        std::function<void(Client *, ClientMsg *)> callback,
        std::function<void(Client *)> disconnectionCallback,
        std::function<void(Client **)> connectionCallback
    );

    template<typename T> void Send(Client *client, const T msg, std::optional<bool> flush = false) {
        ENetPacketFlag flag = ENET_PACKET_FLAG_RELIABLE;

        if constexpr (std::is_same_v<T, ServerMsg_Location>) {
            flag = ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
        }

        ENetPacket* packet = enet_packet_create(reinterpret_cast<const void*>(&msg), sizeof(T), flag);
        enet_peer_send(peers[client], 0, packet);

        if (flush.has_value() && flush.value()) {
            enet_host_flush(server);
        }
    }

    template<typename T> void SendExcept(Client *client, const T msg, std::optional<bool> flush = false) {
        for (const auto &pair : peers) {
            if (pair.first == client) continue;
            Send(pair.first, msg);
        }
    }

    void Close();

    Client *GetClient(ENetPeer *peer);

    // void Event_MsgRecv(ENetEvent event);
    // void Event_Disconnection(ENetEvent event);

    // void MsgRecv_InitClient(ENetEvent event, ClientMsg_Init *msg);
    // void MsgRecv_Location(ENetEvent event, ClientMsg_Location *msg);
    // void MsgRecv_Shot(ENetEvent event, ClientMsg_Shot *msg);
    // void MsgRecv_RequestSpawn(ENetEvent event, ClientMsg_RequestSpawn *msg);
    // void MsgRecv_Spawn(ENetEvent event, ClientMsg_Spawn *msg);

    // void MsgSend_RemoveShot(Shot *shot);
    // void MsgSend_Die(Client *client, Shot *shot);

    // void InitClient(Client *client);
    // void SendToClientsExcept(void *packet, int len, Client *client);
    // void SendToClient(Client *client, void *data, int len);
};