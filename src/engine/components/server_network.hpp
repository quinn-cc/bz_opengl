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

    template<typename T> void send(client_id clientId, const T *input) {
        static_assert(std::is_base_of_v<ServerMsg, T>, "T must be a subclass of ServerMsg");

        if (clientId == BROADCAST_CLIENT_ID) {
            sendAll<T>(input);
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
                msg.set_type(bz::ServerMsg::PLAYER_JOIN);
                auto* join = msg.mutable_player_join();
                join->set_client_id(input->clientId);
                auto* state = join->mutable_state();
                state->set_name(input->state.name);
                state->mutable_position()->set_x(input->state.position.x);
                state->mutable_position()->set_y(input->state.position.y);
                state->mutable_position()->set_z(input->state.position.z);
                state->mutable_rotation()->set_x(input->state.rotation.x);
                state->mutable_rotation()->set_y(input->state.rotation.y);
                state->mutable_rotation()->set_z(input->state.rotation.z);
                state->mutable_rotation()->set_w(input->state.rotation.w);
                state->mutable_velocity()->set_x(input->state.velocity.x);
                state->mutable_velocity()->set_y(input->state.velocity.y);
                state->mutable_velocity()->set_z(input->state.velocity.z);
                state->set_alive(input->state.alive);
            } else if constexpr (std::is_same_v<T, ServerMsg_PlayerLeave>) {
                msg.set_type(bz::ServerMsg::PLAYER_LEAVE);
                auto* leave = msg.mutable_player_leave();
                leave->set_client_id(input->clientId);
            } else if constexpr (std::is_same_v<T, ServerMsg_PlayerState>) {
                msg.set_type(bz::ServerMsg::PLAYER_STATE);
                auto* ps = msg.mutable_player_state();
                ps->set_client_id(input->clientId);
                auto* state = ps->mutable_state();
                state->set_name(input->state.name);
                state->mutable_position()->set_x(input->state.position.x);
                state->mutable_position()->set_y(input->state.position.y);
                state->mutable_position()->set_z(input->state.position.z);
                state->mutable_rotation()->set_x(input->state.rotation.x);
                state->mutable_rotation()->set_y(input->state.rotation.y);
                state->mutable_rotation()->set_z(input->state.rotation.z);
                state->mutable_rotation()->set_w(input->state.rotation.w);
                state->mutable_velocity()->set_x(input->state.velocity.x);
                state->mutable_velocity()->set_y(input->state.velocity.y);
                state->mutable_velocity()->set_z(input->state.velocity.z);
                state->set_alive(input->state.alive);
            } else if constexpr (std::is_same_v<T, ServerMsg_PlayerParameters>) {
                msg.set_type(bz::ServerMsg::PLAYER_PARAMETERS);
                auto* pp = msg.mutable_player_parameters();
                pp->set_client_id(input->clientId);
                auto* params = pp->mutable_params();
                params->set_speed(input->params.speed);
                params->set_turn_speed(input->params.turnSpeed);
                params->set_jump_speed(input->params.jumpSpeed);
                params->set_shot_speed(input->params.shotSpeed);
                params->set_gravity(input->params.gravity);
                params->set_forward_speed_multiplier(input->params.forwardSpeedMultiplier);
                params->set_backward_speed_multiplier(input->params.backwardSpeedMultiplier);
                params->set_left_turn_speed_multiplier(input->params.leftTurnSpeedMultiplier);
                params->set_right_turn_speed_multiplier(input->params.rightTurnSpeedMultiplier);
            } else if constexpr (std::is_same_v<T, ServerMsg_PlayerLocation>) {
                msg.set_type(bz::ServerMsg::PLAYER_LOCATION);
                auto* loc = msg.mutable_player_location();
                loc->set_client_id(input->clientId);
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
                msg.set_type(bz::ServerMsg::PLAYER_SPAWN);
                auto* spawn = msg.mutable_player_spawn();
                spawn->set_client_id(input->clientId);
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
                msg.set_type(bz::ServerMsg::PLAYER_DEATH);
                auto* death = msg.mutable_player_death();
                death->set_client_id(input->clientId);
            } else if constexpr (std::is_same_v<T, ServerMsg_CreateShot>) {
                msg.set_type(bz::ServerMsg::CREATE_SHOT);
                auto* shot = msg.mutable_create_shot();
                shot->set_global_shot_id(input->globalShotId);
                shot->mutable_position()->set_x(input->position.x);
                shot->mutable_position()->set_y(input->position.y);
                shot->mutable_position()->set_z(input->position.z);
                shot->mutable_velocity()->set_x(input->velocity.x);
                shot->mutable_velocity()->set_y(input->velocity.y);
                shot->mutable_velocity()->set_z(input->velocity.z);
            } else if constexpr (std::is_same_v<T, ServerMsg_RemoveShot>) {
                msg.set_type(bz::ServerMsg::REMOVE_SHOT);
                auto* remove = msg.mutable_remove_shot();
                remove->set_shot_id(input->shotId);
                remove->set_is_global_id(input->isGlobalId);
            } else if constexpr (std::is_same_v<T, ServerMsg_Chat>) {
                msg.set_type(bz::ServerMsg::CHAT);
                auto* chat = msg.mutable_chat();
                chat->set_from_id(input->fromId);
                chat->set_to_id(input->toId);
                chat->set_text(input->text);
            } else if constexpr (std::is_same_v<T, ServerMsg_Init>) {
                msg.set_type(bz::ServerMsg::INIT);
                auto* init = msg.mutable_init();
                init->set_client_id(input->clientId);
                init->set_server_name(input->serverName);
                auto* params = init->mutable_default_player_params();
                params->set_speed(input->defaultPlayerParams.speed);
                params->set_turn_speed(input->defaultPlayerParams.turnSpeed);
                params->set_jump_speed(input->defaultPlayerParams.jumpSpeed);
                params->set_shot_speed(input->defaultPlayerParams.shotSpeed);
                params->set_gravity(input->defaultPlayerParams.gravity);
                params->set_forward_speed_multiplier(input->defaultPlayerParams.forwardSpeedMultiplier);
                params->set_backward_speed_multiplier(input->defaultPlayerParams.backwardSpeedMultiplier);
                params->set_left_turn_speed_multiplier(input->defaultPlayerParams.leftTurnSpeedMultiplier);
                params->set_right_turn_speed_multiplier(input->defaultPlayerParams.rightTurnSpeedMultiplier);
                init->set_world_data(input->worldData.data(), input->worldData.size());
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
        }
    };

    template<typename T> void sendExcept(client_id client, const T *input) {
        static_assert(std::is_base_of_v<ServerMsg, T>, "T must be a subclass of ServerMsg");

        for (const auto& [id, peer] : clients) {
            if (id != client) {
                send<T>(id, input);
            }
        }
    };

    template<typename T> void sendAll(const T *input) {
        static_assert(std::is_base_of_v<ServerMsg, T>, "T must be a subclass of ServerMsg");

        for (const auto& [id, peer] : clients) {
            send<T>(id, input);
        }
    };

    std::vector<client_id> getClients() const;
};