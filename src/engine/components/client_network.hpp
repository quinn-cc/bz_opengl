#pragma once
#include "engine/types.hpp"
#include <string>
#include <cstdint>
#include <enet.h>
#include "messages.pb.h"
#include "spdlog/spdlog.h"

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

    template<typename T> void send(const T &input, bool flush = false) {
        static_assert(std::is_base_of_v<ClientMsg, T>, "T must be a subclass of ClientMsg");

        ENetPacketFlag flag = ENET_PACKET_FLAG_RELIABLE;

        if constexpr (std::is_same_v<T, ClientMsg_PlayerLocation>) {
            flag = ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
        }

        bz::ClientMsg msg;
        msg.set_client_id(input.clientId);

        if constexpr (std::is_same_v<T, ClientMsg_Init>) {
            msg.set_type(bz::ClientMsg::INIT);
            msg.mutable_init()->set_name(input.name);
        } else if constexpr (std::is_same_v<T, ClientMsg_Chat>) {
            msg.set_type(bz::ClientMsg::CHAT);
            auto* chat = msg.mutable_chat();
            chat->set_to_id(input.toId);
            chat->set_text(input.text);
        } else if constexpr (std::is_same_v<T, ClientMsg_PlayerLocation>) {
            msg.set_type(bz::ClientMsg::PLAYER_LOCATION);
            auto* loc = msg.mutable_player_location();
            loc->mutable_position()->set_x(input.position.x);
            loc->mutable_position()->set_y(input.position.y);
            loc->mutable_position()->set_z(input.position.z);
            loc->mutable_rotation()->set_w(input.rotation.w);
            loc->mutable_rotation()->set_x(input.rotation.x);
            loc->mutable_rotation()->set_y(input.rotation.y);
            loc->mutable_rotation()->set_z(input.rotation.z);
        } else if constexpr (std::is_same_v<T, ClientMsg_RequestPlayerSpawn>) {
            msg.set_type(bz::ClientMsg::REQUEST_PLAYER_SPAWN);
            msg.mutable_request_player_spawn();
        } else if constexpr (std::is_same_v<T, ClientMsg_CreateShot>) {
            msg.set_type(bz::ClientMsg::CREATE_SHOT);
            auto* shot = msg.mutable_create_shot();
            shot->set_local_shot_id(input.localShotId);
            shot->mutable_position()->set_x(input.position.x);
            shot->mutable_position()->set_y(input.position.y);
            shot->mutable_position()->set_z(input.position.z);
            shot->mutable_velocity()->set_x(input.velocity.x);
            shot->mutable_velocity()->set_y(input.velocity.y);
            shot->mutable_velocity()->set_z(input.velocity.z);
        } else if constexpr (std::is_same_v<T, ClientMsg_PlayerJoin>) {
            msg.set_type(bz::ClientMsg::PLAYER_JOIN);
            msg.mutable_player_join()->set_ip(input.ip);
        } else if constexpr (std::is_same_v<T, ClientMsg_PlayerLeave>) {
            msg.set_type(bz::ClientMsg::PLAYER_LEAVE);
            msg.mutable_player_leave();
        } else {
            spdlog::error("ClientNetwork::send: Unsupported message type");
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