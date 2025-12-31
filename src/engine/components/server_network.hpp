#pragma once
#include "engine/types.hpp"
#include <string>
#include <cstdint>
#include <enet.h>
#include <optional>
#include <map>
#include <functional>
#include "spdlog/spdlog.h"

class ServerNetwork {
    friend class ServerEngine;

private:
    ENetHost *server;
    std::map<client_id, ENetPeer *> clients;

    struct MsgData {
        ENetPacket* packet;
        ClientMsg* msg;
        bool peeked = false;
    };

    std::vector<struct MsgData> receivedMessages;

    ServerNetwork(
        uint16_t port,
        int maxClients = 50,
        int numChannels = 2
    );
    ~ServerNetwork();

    void flushPeekedMessages();
    client_id getClient(ENetPeer *peer);
    client_id getNextClientId();
    void update();

public:
    template<typename T> T* peekMessage(std::function<bool(const T&)> predicate = [](const T&) { return true; }) {
        static_assert(std::is_base_of_v<ClientMsg, T>, "T must be a subclass of ClientMsg");

        for (auto &msgData : receivedMessages) {
            // Check if the message is of type ClientMsg or if it is of type T
            if (msgData.msg->type == T::Type) {
                auto* casted = static_cast<T*>(msgData.msg);

                if (predicate(*casted)) {
                    msgData.peeked = true;
                    return casted;
                }
            }
        }

        return nullptr;
    };

    template<typename T> void send(client_id clientId, const T *msg, bool flush = false) {
        static_assert(std::is_base_of_v<ServerMsg, T>, "T must be a subclass of ServerMsg");

        if (clientId == BROADCAST_CLIENT_ID) {
            sendAll<T>(msg, flush);
            return;
        }

        if (clients.find(clientId) == clients.end()) {
            spdlog::debug("ServerNetwork::send: Attempted to send message to non-existent client id {}", clientId);
        } else {
            ENetPeer *client = clients[clientId];
            ENetPacketFlag flag = ENET_PACKET_FLAG_RELIABLE;

            if constexpr (std::is_same_v<T, ClientMsg_Location>) {
                flag = ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
            }
            
            ENetPacket* packet;

            if constexpr (std::is_same_v<T, ServerMsg_Init>) {
                size_t totalSize = sizeof(ServerMsg_Init) - sizeof(std::byte) + msg->dataSize;
                packet = enet_packet_create(reinterpret_cast<const void*>(msg), totalSize, flag);
            } else {
                packet = enet_packet_create(reinterpret_cast<const void*>(msg), sizeof(T), flag);
            }
            enet_peer_send(client, 0, packet);

            if (flush) {
                enet_host_flush(server);
            }
        }
    };

    template<typename T> void sendExcept(client_id client, const T *msg, bool flush = false) {
        static_assert(std::is_base_of_v<ServerMsg, T>, "T must be a subclass of ServerMsg");

        for (const auto& [id, peer] : clients) {
            if (id != client) {
                send<T>(id, msg, flush);
            }
        }
    };

    template<typename T> void sendAll(const T *msg, bool flush = false) {
        static_assert(std::is_base_of_v<ServerMsg, T>, "T must be a subclass of ServerMsg");

        for (const auto& [id, peer] : clients) {
            send<T>(id, msg, flush);
        }
    };

    std::vector<client_id> getClients() const;
};