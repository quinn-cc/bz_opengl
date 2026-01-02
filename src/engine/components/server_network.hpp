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

    template<typename T> void send(client_id clientId, const T *input, bool flush = false) {
        static_assert(std::is_base_of_v<ServerMsg, T>, "T must be a subclass of ServerMsg");

        if (clientId == BROADCAST_CLIENT_ID) {
            sendAll<T>(input, flush);
            return;
        }

        if (clients.find(clientId) == clients.end()) {
            spdlog::debug("ServerNetwork::send: Attempted to send message to non-existent client id {}", clientId);
        } else {
            ENetPeer *peer = clients[clientId];
            ENetPacketFlag flag = ENET_PACKET_FLAG_RELIABLE;

            if constexpr (std::is_same_v<T, ServerMsg_PlayerLocation>) {
                flag = ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
            }
            
            bz::ServerMsg msg;

            if constexpr (std::is_same_v<T, ServerMsg_PlayerJoin>) {
                auto* join = msg.mutable_player_join();
                join->set_clientid(input->clientId);
                join->set_name(input->name);
                // TODO: set state properly
            } else if constexpr (std::is_same_v<T, ServerMsg_PlayerLeave>) {
                auto* leave = msg.mutable_player_leave();
                leave->set_clientid(input->clientId);
            } else if constexpr (std::is_same_v<T, ServerMsg_PlayerState>) {
                auto* state = msg.mutable_player_state();
                state->set_clientid(input->clientId);
                // TODO: set state properly
            } else if constexpr (std::is_same_v<T, ServerMsg_DefaultPlayerStateChange>) {
                auto* change = msg.mutable_default_player_state_change();
                change->set_clientid(input->clientId);
                change->set_key(input->key);
                change->set_value(input->value);
            } else if constexpr (std::is_same_v<T, ServerMsg_PlayerLocation>) {
                auto* loc = msg.mutable_player_location();
                loc->set_clientid(input->clientId);
                loc->mutable_position()->set_x(input->position.x);
                loc->mutable_position()->set_y(input->position.y);
                loc->mutable_position()->set_z(input->position.z);
                loc->mutable_rotation()->set_x(input->rotation.x);
                loc->mutable_rotation()->set_y(input->rotation.y);
                loc->mutable_rotation()->set_z(input->rotation.z);
                loc->mutable_rotation()->set_w(input->rotation.w);
                loc->mutable_velocity()->set_x(input->velocity.x);
                loc->mutable_velocity()->set_y(input->velocity.y);
                loc->mutable_velocity()->set_z(input->velocity.z);
            } else if constexpr (std::is_same_v<T, ServerMsg_PlayerSpawn>) {
                auto* spawn = msg.mutable_player_spawn();
                spawn->set_clientid(input->clientId);
                spawn->mutable_position()->set_x(input->position.x);
                spawn->mutable_position()->set_y(input->position.y);
                spawn->mutable_position()->set_z(input->position.z);
                spawn->mutable_rotation()->set_x(input->rotation.x);
                spawn->mutable_rotation()->set_y(input->rotation.y);
                spawn->mutable_rotation()->set_z(input->rotation.z);
                spawn->mutable_rotation()->set_w(input->rotation.w);
                spawn->mutable_velocity()->set_x(input->velocity.x);
                spawn->mutable_velocity()->set_y(input->velocity.y);
                spawn->mutable_velocity()->set_z(input->velocity.z);
            } else if constexpr (std::is_same_v<T, ServerMsg_PlayerDeath>) {
                auto* death = msg.mutable_player_death();
                death->set_clientid(input->clientId);
            } else if constexpr (std::is_same_v<T, ServerMsg_CreateShot>) {
                auto* shot = msg.mutable_create_shot();
                shot->set_globalshotid(input->globalShotId);
                shot->mutable_position()->set_x(input->position.x);
                shot->mutable_position()->set_y(input->position.y);
                shot->mutable_position()->set_z(input->position.z);
                shot->mutable_velocity()->set_x(input->velocity.x);
                shot->mutable_velocity()->set_y(input->velocity.y);
                shot->mutable_velocity()->set_z(input->velocity.z);
            } else if constexpr (std::is_same_v<T, ServerMsg_RemoveShot>) {
                auto* remove = msg.mutable_remove_shot();
                remove->set_shotid(input->shotId);
                remove->set_isglobalid(input->isGlobalId);
            } else if constexpr (std::is_same_v<T, ServerMsg_Chat>) {
                auto* chat = msg.mutable_chat();
                chat->set_fromid(input->fromId);
                chat->set_toid(input->toId);
                chat->set_text(input->text);
            } else if constexpr (std::is_same_v<T, ServerMsg_Init>) {
                auto* init = msg.mutable_init();
                init->set_clientid(input->clientId);
                init->set_servername(input->serverName);
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

    template<typename T> void sendExcept(client_id client, const T *input, bool flush = false) {
        static_assert(std::is_base_of_v<ServerMsg, T>, "T must be a subclass of ServerMsg");

        for (const auto& [id, peer] : clients) {
            if (id != client) {
                send<T>(id, input, flush);
            }
        }
    };

    template<typename T> void sendAll(const T *input, bool flush = false) {
        static_assert(std::is_base_of_v<ServerMsg, T>, "T must be a subclass of ServerMsg");

        for (const auto& [id, peer] : clients) {
            send<T>(id, input, flush);
        }
    };

    std::vector<client_id> getClients() const;
};