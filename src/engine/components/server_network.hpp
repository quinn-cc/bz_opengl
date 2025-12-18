#pragma once
#include "engine/types.hpp"
#include <string>
#include <cstdint>
#include <enet.h>
#include <optional>
#include <map>
#include <functional>

class ServerNetwork {
    friend class ServerEngine;

private:
    ENetHost *server;
    std::map<client_id, ENetPeer *> clients;
    std::vector<ClientMsg*> receivedMessages;
    std::function<void(client_id)> connectionCallback;
    std::function<void(client_id)> disconnectionCallback;

    ServerNetwork(
        uint16_t port,
        std::function<void(client_id)> connectionCallback,
        std::function<void(client_id)> disconnectionCallback,
        int maxClients = 50,
        int numChannels = 2
    );
    ~ServerNetwork();

    client_id getClient(ENetPeer *peer);
    client_id getNextClientId();
    void update();

public:
    template<typename T> std::optional<T*> peek() const {
        static_assert(std::is_base_of_v<ClientMsg, T>, "T must be a subclass of ClientMsg");

        for (auto msg : receivedMessages) {
            if (msg->type == T::Type) {
                return reinterpret_cast<T*>(msg);
            }
        }

        return std::nullopt;
    };

    template<typename T> void send(client_id clientId, const T &msg, bool flush = false) {
        static_assert(std::is_base_of_v<ClientMsg, T>, "T must be a subclass of ClientMsg");

        ENetPeer *client = clients[clientId];
        ENetPacketFlag flag = ENET_PACKET_FLAG_RELIABLE;

        if constexpr (std::is_same_v<T, ClientMsg_Location>) {
            flag = ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
        }

        ENetPacket* packet = enet_packet_create(reinterpret_cast<const void*>(&msg), sizeof(T), flag);
        enet_peer_send(client, 0, packet);

        if (flush) {
            enet_host_flush(server);
        }
    };

    template<typename T> void sendExcept(client_id client, const T &msg, bool flush = false) {
        static_assert(std::is_base_of_v<ClientMsg, T>, "T must be a subclass of ClientMsg");

        for (const auto& [id, peer] : clients) {
            if (id != client) {
                send<T>(peer, msg, flush);
            }
        }
    };

    void pop(ClientMsg* msg);
    std::vector<client_id> getClients() const;
};