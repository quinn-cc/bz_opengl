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

    template<typename T> void send(const T &msg, bool flush = false) {
        static_assert(std::is_base_of_v<ClientMsg, T>, "T must be a subclass of ClientMsg");

        ENetPacketFlag flag = ENET_PACKET_FLAG_RELIABLE;

        if constexpr (std::is_same_v<T, ClientMsg_Location>) {
            flag = ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
        }

        ENetPacket* packet;
        bz::ServerMsg msg;

        if (constexpr (std::is_same_v<T, ClientMsg_Init)) {
            msg.mutable_init_msg();
            msg->set_name(msg->name);
        } else if (constexpr (std::is_same_v<T, ClientMsg_Chat)) {
            msg.mutable_chat_msg();
            msg->set_toid(msg->toId);
            msg->set_text(msg->text);
        } else if (constexpr (std::is_same_v<T, ClientMsg_Location)) {
            msg.mutable_location_msg();
            msg->mutable_location_msg()->mutable_position()->set_x(msg->position.x);
            msg.mutable_location_msg()->mutable_position()->set_y(msg->position.y);
            msg.mutable_location_msg()->mutable_position()->set_z(msg->position.z);
            msg.mutable_location_msg()->mutable_rotation()->set_w(msg->rotation.w);
            msg.mutable_location_msg()->mutable_rotation()->set_x(msg->rotation.x);
            msg.mutable_location_msg()->mutable_rotation()->set_y(msg->rotation.y);
            msg.mutable_location_msg()->mutable_rotation()->set_z(msg->rotation.z);
        } else if (constexpr (std::is_same_v<T, ClientMsg_RequestSpawn)) {
            msg.mutable_request_spawn_msg();
        } else if (constexpr (std::is_same_v<T, ClientMsg_CreateShot>)) {
            msg.mutable_create_shot_msg();
            msg->mutable_create_shot_msg()->set_localshotid(msg->localShotId);
            msg->mutable_create_shot_msg()->mutable_position()->set_x(msg->position.x);
            msg->mutable_create_shot_msg()->mutable_position()->set_y(msg->position.y);
            msg->mutable_create_shot_msg()->mutable_position()->set_z(msg->position.z);
            msg->mutable_create_shot_msg()->mutable_velocity()->set_x(msg->velocity.x);
            msg->mutable_create_shot_msg()->mutable_velocity()->set_y(msg->velocity.y);
            msg->mutable_create_shot_msg()->mutable_velocity()->set_z(msg->velocity.z);
        } else {
            spdlog::error("ServerNetwork::send: Unsupported message type");
            return;
        }
        
        std::string buffer;
        msg.SerializeToString(&buffer);

        ENetPacket* packet = enet_packet_create(
            buffer.data(),
            buffer.size(),
            flag
        );

        enet_peer_send(server, 0, packet);

        if (flush) {
            enet_host_flush(client);
        }
    };
};