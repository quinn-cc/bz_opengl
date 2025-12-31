#pragma once
#include "engine/types.hpp"
#include <string>
#include <cstdint>
#include <enet.h>
#include "messages.pb.h"

class ClientNetwork {
    friend class ClientEngine;

private:
    ENetHost* client = nullptr;
    ENetPeer* server = nullptr;

    struct MsgData {
        ENetPacket* packet;
        ServerMsg* msg;
        bool peeked = false;
    };

    std::vector<struct MsgData> receivedMessages;

    ClientNetwork();
    ~ClientNetwork();

    void flushPeekedMessages();
    void update();

public:
    bool connect(const std::string &address, uint16_t port, int timeoutMs = 5000);

    template<typename T> T* peekMessage(std::function<bool(const T&)> predicate = [](const T&) { return true; }) {
        static_assert(std::is_base_of_v<ServerMsg, T>, "T must be a subclass of ServerMsg");

        for (auto &msgData : receivedMessages) {
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

    void sendInit(const std::string &playerName);
    void sendLocation(const Location &location);
    void sendChat(const std::string &text, client_id toId = BROADCAST_CLIENT_ID);
    void sendRequestSpawn();
    void sendCreateShot(const glm::vec3 &position, const glm::vec3 &velocity);

    bool receiveLocation(client_id id, const Location &location);
    bool receivePlayerJoin(client_id id, const std::string &name, const Location &location, bool alive);
    bool receivePlayerState(client_id id, SettingsMap settings);
    bool receivePlayerLeave(client_id id);
    bool receiveChat(client_id &fromId, client_id &toId, const std::string &text);
    bool receiveCreateShot(client_id &ownerId, uint32_t &globalShotId, const glm::vec3 &position, const glm::vec3 &velocity);
    bool receiveRemoveShot(shot_id id, bool &isGlobalId);
    bool receiveSpawn(client_id id, const Location &location);
    bool receiveDeath(client_id id);

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