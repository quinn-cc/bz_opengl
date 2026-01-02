#pragma once
#include "engine/types.hpp"
#include <string>
#include <cstdint>
#include <enet.h>
#include <optional>
#include <map>
#include <functional>
#include "spdlog/spdlog.h"
#include "messages.pb.h"

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
            bz::ServerMsg msg;

            if (constexpr (std::is_same_v<T, ServerMsg_PlayerJoin>)) {
                msg.mutable_init_msg();
                msg->set_clientid(msg->clientId);
                msg->set_name(msg->name);
                msg->set_state(msg->state);
            } else if (constexpr (std::is_same_v<T, ServerMsg_PlayerLeave>)) {
                msg.mutable_leave_msg();
                msg->set_clientid(msg->clientId);
            } else if (constexpr (std::is_same_v<T, ServerMsg_PlayerState>)) {
                msg.mutable_state_msg();
                msg->set_clientid(msg->clientId);
                msg->set_state(msg->state);
            } else if (constexpr (std::is_same_v<T, ServerMsg_DefaultPlayerStateChange>)) {
                msg.mutable_default_player_state_change_msg();
                msg->set_clientid(msg->clientId);
                msg->set_key(msg->key);
                msg->set_value(msg->value);
            } else if (constexpr (std::is_same_v<T, ServerMsg_PlayerLocation>)) {
                msg.mutable_location_msg();
                msg->set_clientid(msg->clientId);
                auto* loc = msg.mutable_location_msg();
                loc->mutable_position()->set_x(msg->position.x);
                loc->mutable_position()->set_y(msg->position.y);
                loc->mutable_position()->set_z(msg->position.z);
                loc->mutable_rotation()->set_x(msg->rotation.x);
                loc->mutable_rotation()->set_y(msg->rotation.y);
                loc->mutable_rotation()->set_z(msg->rotation.z);
                loc->mutable_rotation()->set_w(msg->rotation.w);
                loc->mutable_velocity()->set_x(msg->velocity.x);
                loc->mutable_velocity()->set_y(msg->velocity.y);
                loc->mutable_velocity()->set_z(msg->velocity.z);
            } else if (constexpr (std::is_same_v<T, ServerMsg_PlayerSpawn>)) {
                msg.mutable_spawn_msg();
                msg->set_clientid(msg->clientId);
                auto* spawn = msg.mutable_spawn_msg();
                spawn->mutable_position()->set_x(msg->position.x);
                spawn->mutable_position()->set_y(msg->position.y);
                spawn->mutable_position()->set_z(msg->position.z);
                spawn->mutable_rotation()->set_x(msg->rotation.x);
                spawn->mutable_rotation()->set_y(msg->rotation.y);
                spawn->mutable_rotation()->set_z(msg->rotation.z);
                spawn->mutable_rotation()->set_w(msg->rotation.w);
                spawn->mutable_velocity()->set_x(msg->velocity.x);
                spawn->mutable_velocity()->set_y(msg->velocity.y);
                spawn->mutable_velocity()->set_z(msg->velocity.z);
            } else if (constexpr (std::is_same_v<T, ServerMsg_PlayerDeath>)) {
                msg.mutable_death_msg();
                msg->set_clientid(msg->clientId);
            } else if (constexpr (std::is_same_v<T, ServerMsg_CreateShot>)) {
                msg.mutable_create_shot_msg();
                msg->set_clientid(msg->clientId);
                auto* shot = msg.mutable_create_shot_msg();
                shot->set_localshotid(msg->localShotId);
                shot->mutable_position()->set_x(msg->position.x);
                shot->mutable_position()->set_y(msg->position.y);
                shot->mutable_position()->set_z(msg->position.z);
                shot->mutable_velocity()->set_x(msg->velocity.x);
                shot->mutable_velocity()->set_y(msg->velocity.y);
                shot->mutable_velocity()->set_z(msg->velocity.z);
            } else if (constexpr (std::is_same_v<T, ServerMsg_RemoveShot>)) {
                msg.mutable_remove_shot_msg();
                msg->set_globalshotid(msg->globalShotId);
            } else if (constexpr (std::is_same_v<T, ServerMsg_Chat>)) {
                msg.mutable_chat_msg();
                msg->set_fromid(msg->fromId);
                msg->set_toid(msg->toId);
                msg->set_text(msg->text);
            } else if (constexpr (std::is_same_v<T, ServerMsg_Init>)) {
                msg.mutable_init_msg();
                msg->set_clientid(msg->clientId);
                msg->set_servername(msg->serverName);
            }
            else {
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

            enet_peer_send(peer, 0, packet);

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