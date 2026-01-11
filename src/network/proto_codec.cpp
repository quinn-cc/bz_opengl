#include "network/proto_codec.hpp"

#include "messages.pb.h"

#include <string>

namespace net {
namespace {

std::vector<std::byte> toBytes(const std::string &buffer) {
    const auto *ptr = reinterpret_cast<const std::byte*>(buffer.data());
    return std::vector<std::byte>(ptr, ptr + buffer.size());
}

} // namespace

std::unique_ptr<ServerMsg> decodeServerMsg(const std::byte *data, std::size_t size) {
    if (!data || size == 0) {
        return nullptr;
    }

    bz::ServerMsg msg;
    if (!msg.ParseFromArray(data, static_cast<int>(size))) {
        return nullptr;
    }

    switch (msg.payload_case()) {

    case bz::ServerMsg::kPlayerJoin: {
        auto out = std::make_unique<ServerMsg_PlayerJoin>();
        out->clientId = msg.player_join().client_id();
        out->state.name = msg.player_join().state().name();
        out->state.position.x = msg.player_join().state().position().x();
        out->state.position.y = msg.player_join().state().position().y();
        out->state.position.z = msg.player_join().state().position().z();
        out->state.rotation.w = msg.player_join().state().rotation().w();
        out->state.rotation.x = msg.player_join().state().rotation().x();
        out->state.rotation.y = msg.player_join().state().rotation().y();
        out->state.rotation.z = msg.player_join().state().rotation().z();
        out->state.velocity.x = msg.player_join().state().velocity().x();
        out->state.velocity.y = msg.player_join().state().velocity().y();
        out->state.velocity.z = msg.player_join().state().velocity().z();
        out->state.alive = msg.player_join().state().alive();
        return out;
    }

    case bz::ServerMsg::kPlayerLeave: {
        auto out = std::make_unique<ServerMsg_PlayerLeave>();
        out->clientId = msg.player_leave().client_id();
        return out;
    }

    case bz::ServerMsg::kPlayerState: {
        auto out = std::make_unique<ServerMsg_PlayerState>();
        out->clientId = msg.player_state().client_id();
        out->state.name = msg.player_state().state().name();
        out->state.position.x = msg.player_state().state().position().x();
        out->state.position.y = msg.player_state().state().position().y();
        out->state.position.z = msg.player_state().state().position().z();
        out->state.rotation.w = msg.player_state().state().rotation().w();
        out->state.rotation.x = msg.player_state().state().rotation().x();
        out->state.rotation.y = msg.player_state().state().rotation().y();
        out->state.rotation.z = msg.player_state().state().rotation().z();
        out->state.velocity.x = msg.player_state().state().velocity().x();
        out->state.velocity.y = msg.player_state().state().velocity().y();
        out->state.velocity.z = msg.player_state().state().velocity().z();
        out->state.alive = msg.player_state().state().alive();
        return out;
    }

    case bz::ServerMsg::kPlayerParameters: {
        auto out = std::make_unique<ServerMsg_PlayerParameters>();
        out->clientId = msg.player_parameters().client_id();
        for (const auto& [key, val] : msg.player_parameters().params().params()) {
            out->params[key] = val;
        }
        return out;
    }

    case bz::ServerMsg::kPlayerLocation: {
        auto out = std::make_unique<ServerMsg_PlayerLocation>();
        out->clientId = msg.player_location().client_id();
        out->position.x = msg.player_location().position().x();
        out->position.y = msg.player_location().position().y();
        out->position.z = msg.player_location().position().z();
        out->rotation.w = msg.player_location().rotation().w();
        out->rotation.x = msg.player_location().rotation().x();
        out->rotation.y = msg.player_location().rotation().y();
        out->rotation.z = msg.player_location().rotation().z();
        out->velocity.x = msg.player_location().velocity().x();
        out->velocity.y = msg.player_location().velocity().y();
        out->velocity.z = msg.player_location().velocity().z();
        return out;
    }

    case bz::ServerMsg::kPlayerSpawn: {
        auto out = std::make_unique<ServerMsg_PlayerSpawn>();
        out->clientId = msg.player_spawn().client_id();
        out->position.x = msg.player_spawn().position().x();
        out->position.y = msg.player_spawn().position().y();
        out->position.z = msg.player_spawn().position().z();
        out->rotation.w = msg.player_spawn().rotation().w();
        out->rotation.x = msg.player_spawn().rotation().x();
        out->rotation.y = msg.player_spawn().rotation().y();
        out->rotation.z = msg.player_spawn().rotation().z();
        out->velocity.x = msg.player_spawn().velocity().x();
        out->velocity.y = msg.player_spawn().velocity().y();
        out->velocity.z = msg.player_spawn().velocity().z();
        return out;
    }

    case bz::ServerMsg::kPlayerDeath: {
        auto out = std::make_unique<ServerMsg_PlayerDeath>();
        out->clientId = msg.player_death().client_id();
        return out;
    }

    case bz::ServerMsg::kCreateShot: {
        auto out = std::make_unique<ServerMsg_CreateShot>();
        out->globalShotId = msg.create_shot().global_shot_id();
        out->position.x = msg.create_shot().position().x();
        out->position.y = msg.create_shot().position().y();
        out->position.z = msg.create_shot().position().z();
        out->velocity.x = msg.create_shot().velocity().x();
        out->velocity.y = msg.create_shot().velocity().y();
        out->velocity.z = msg.create_shot().velocity().z();
        return out;
    }

    case bz::ServerMsg::kRemoveShot: {
        auto out = std::make_unique<ServerMsg_RemoveShot>();
        out->shotId = msg.remove_shot().shot_id();
        out->isGlobalId = msg.remove_shot().is_global_id();
        return out;
    }

    case bz::ServerMsg::kInit: {
        auto out = std::make_unique<ServerMsg_Init>();
        out->clientId = msg.init().client_id();
        out->serverName = msg.init().server_name();
        for (const auto& [key, val] : msg.init().default_player_params().params()) {
            out->defaultPlayerParams[key] = val;
        }

        const std::string &worldDataStr = msg.init().world_data();
        const auto *dataPtr = reinterpret_cast<const std::byte*>(worldDataStr.data());
        out->worldData = std::vector<std::byte>(dataPtr, dataPtr + worldDataStr.size());
        return out;
    }

    case bz::ServerMsg::kChat: {
        auto out = std::make_unique<ServerMsg_Chat>();
        out->fromId = msg.chat().from_id();
        out->toId = msg.chat().to_id();
        out->text = msg.chat().text();
        return out;
    }

    default:
        return nullptr;
    }
}

std::unique_ptr<ClientMsg> decodeClientMsg(const std::byte *data, std::size_t size) {
    if (!data || size == 0) {
        return nullptr;
    }

    bz::ClientMsg msg;
    if (!msg.ParseFromArray(data, static_cast<int>(size))) {
        return nullptr;
    }

    switch (msg.payload_case()) {

    case bz::ClientMsg::kInit: {
        auto out = std::make_unique<ClientMsg_Init>();
        out->clientId = msg.client_id();
        out->name = msg.init().name();
        return out;
    }

    case bz::ClientMsg::kChat: {
        auto out = std::make_unique<ClientMsg_Chat>();
        out->clientId = msg.client_id();
        out->toId = msg.chat().to_id();
        out->text = msg.chat().text();
        return out;
    }

    case bz::ClientMsg::kPlayerLocation: {
        auto out = std::make_unique<ClientMsg_PlayerLocation>();
        out->clientId = msg.client_id();
        out->position.x = msg.player_location().position().x();
        out->position.y = msg.player_location().position().y();
        out->position.z = msg.player_location().position().z();
        out->rotation.w = msg.player_location().rotation().w();
        out->rotation.x = msg.player_location().rotation().x();
        out->rotation.y = msg.player_location().rotation().y();
        out->rotation.z = msg.player_location().rotation().z();
        return out;
    }

    case bz::ClientMsg::kRequestPlayerSpawn: {
        auto out = std::make_unique<ClientMsg_RequestPlayerSpawn>();
        out->clientId = msg.client_id();
        return out;
    }

    case bz::ClientMsg::kCreateShot: {
        auto out = std::make_unique<ClientMsg_CreateShot>();
        out->clientId = msg.client_id();
        out->localShotId = msg.create_shot().local_shot_id();
        out->position.x = msg.create_shot().position().x();
        out->position.y = msg.create_shot().position().y();
        out->position.z = msg.create_shot().position().z();
        out->velocity.x = msg.create_shot().velocity().x();
        out->velocity.y = msg.create_shot().velocity().y();
        out->velocity.z = msg.create_shot().velocity().z();
        return out;
    }

    case bz::ClientMsg::kPlayerJoin: {
        auto out = std::make_unique<ClientMsg_PlayerJoin>();
        out->clientId = msg.client_id();
        out->ip = msg.player_join().ip();
        return out;
    }

    case bz::ClientMsg::kPlayerLeave: {
        auto out = std::make_unique<ClientMsg_PlayerLeave>();
        out->clientId = msg.client_id();
        return out;
    }

    default:
        return nullptr;
    }
}

std::optional<std::vector<std::byte>> encodeClientMsg(const ClientMsg &input) {
    bz::ClientMsg msg;
    msg.set_client_id(input.clientId);

    switch (input.type) {
    case ClientMsg_Type_INIT: {
        msg.set_type(bz::ClientMsg::INIT);
        const auto &typed = static_cast<const ClientMsg_Init&>(input);
        msg.mutable_init()->set_name(typed.name);
        break;
    }
    case ClientMsg_Type_CHAT: {
        msg.set_type(bz::ClientMsg::CHAT);
        const auto &typed = static_cast<const ClientMsg_Chat&>(input);
        auto* chat = msg.mutable_chat();
        chat->set_to_id(typed.toId);
        chat->set_text(typed.text);
        break;
    }
    case ClientMsg_Type_PLAYER_LOCATION: {
        msg.set_type(bz::ClientMsg::PLAYER_LOCATION);
        const auto &typed = static_cast<const ClientMsg_PlayerLocation&>(input);
        auto* loc = msg.mutable_player_location();
        loc->mutable_position()->set_x(typed.position.x);
        loc->mutable_position()->set_y(typed.position.y);
        loc->mutable_position()->set_z(typed.position.z);
        loc->mutable_rotation()->set_w(typed.rotation.w);
        loc->mutable_rotation()->set_x(typed.rotation.x);
        loc->mutable_rotation()->set_y(typed.rotation.y);
        loc->mutable_rotation()->set_z(typed.rotation.z);
        break;
    }
    case ClientMsg_Type_REQUEST_PLAYER_SPAWN: {
        msg.set_type(bz::ClientMsg::REQUEST_PLAYER_SPAWN);
        msg.mutable_request_player_spawn();
        break;
    }
    case ClientMsg_Type_CREATE_SHOT: {
        msg.set_type(bz::ClientMsg::CREATE_SHOT);
        const auto &typed = static_cast<const ClientMsg_CreateShot&>(input);
        auto* shot = msg.mutable_create_shot();
        shot->set_local_shot_id(typed.localShotId);
        shot->mutable_position()->set_x(typed.position.x);
        shot->mutable_position()->set_y(typed.position.y);
        shot->mutable_position()->set_z(typed.position.z);
        shot->mutable_velocity()->set_x(typed.velocity.x);
        shot->mutable_velocity()->set_y(typed.velocity.y);
        shot->mutable_velocity()->set_z(typed.velocity.z);
        break;
    }
    case ClientMsg_Type_PLAYER_JOIN: {
        msg.set_type(bz::ClientMsg::PLAYER_JOIN);
        const auto &typed = static_cast<const ClientMsg_PlayerJoin&>(input);
        msg.mutable_player_join()->set_ip(typed.ip);
        break;
    }
    case ClientMsg_Type_PLAYER_LEAVE: {
        msg.set_type(bz::ClientMsg::PLAYER_LEAVE);
        msg.mutable_player_leave();
        break;
    }
    default:
        return std::nullopt;
    }

    std::string buffer;
    msg.SerializeToString(&buffer);
    return toBytes(buffer);
}

std::optional<std::vector<std::byte>> encodeServerMsg(const ServerMsg &input) {
    bz::ServerMsg msg;

    switch (input.type) {
    case ServerMsg_Type_PLAYER_JOIN: {
        msg.set_type(bz::ServerMsg::PLAYER_JOIN);
        const auto &typed = static_cast<const ServerMsg_PlayerJoin&>(input);
        auto* join = msg.mutable_player_join();
        join->set_client_id(typed.clientId);
        auto* state = join->mutable_state();
        state->set_name(typed.state.name);
        state->mutable_position()->set_x(typed.state.position.x);
        state->mutable_position()->set_y(typed.state.position.y);
        state->mutable_position()->set_z(typed.state.position.z);
        state->mutable_rotation()->set_x(typed.state.rotation.x);
        state->mutable_rotation()->set_y(typed.state.rotation.y);
        state->mutable_rotation()->set_z(typed.state.rotation.z);
        state->mutable_rotation()->set_w(typed.state.rotation.w);
        state->mutable_velocity()->set_x(typed.state.velocity.x);
        state->mutable_velocity()->set_y(typed.state.velocity.y);
        state->mutable_velocity()->set_z(typed.state.velocity.z);
        state->set_alive(typed.state.alive);
        break;
    }
    case ServerMsg_Type_PLAYER_LEAVE: {
        msg.set_type(bz::ServerMsg::PLAYER_LEAVE);
        const auto &typed = static_cast<const ServerMsg_PlayerLeave&>(input);
        msg.mutable_player_leave()->set_client_id(typed.clientId);
        break;
    }
    case ServerMsg_Type_PLAYER_STATE: {
        msg.set_type(bz::ServerMsg::PLAYER_STATE);
        const auto &typed = static_cast<const ServerMsg_PlayerState&>(input);
        auto* ps = msg.mutable_player_state();
        ps->set_client_id(typed.clientId);
        auto* state = ps->mutable_state();
        state->set_name(typed.state.name);
        state->mutable_position()->set_x(typed.state.position.x);
        state->mutable_position()->set_y(typed.state.position.y);
        state->mutable_position()->set_z(typed.state.position.z);
        state->mutable_rotation()->set_x(typed.state.rotation.x);
        state->mutable_rotation()->set_y(typed.state.rotation.y);
        state->mutable_rotation()->set_z(typed.state.rotation.z);
        state->mutable_rotation()->set_w(typed.state.rotation.w);
        state->mutable_velocity()->set_x(typed.state.velocity.x);
        state->mutable_velocity()->set_y(typed.state.velocity.y);
        state->mutable_velocity()->set_z(typed.state.velocity.z);
        state->set_alive(typed.state.alive);
        break;
    }
    case ServerMsg_Type_PLAYER_PARAMETERS: {
        msg.set_type(bz::ServerMsg::PLAYER_PARAMETERS);
        const auto &typed = static_cast<const ServerMsg_PlayerParameters&>(input);
        auto* pp = msg.mutable_player_parameters();
        pp->set_client_id(typed.clientId);
        auto* params = pp->mutable_params();
        for (const auto& [key, val] : typed.params) {
            (*params->mutable_params())[key] = val;
        }
        break;
    }
    case ServerMsg_Type_PLAYER_LOCATION: {
        msg.set_type(bz::ServerMsg::PLAYER_LOCATION);
        const auto &typed = static_cast<const ServerMsg_PlayerLocation&>(input);
        auto* loc = msg.mutable_player_location();
        loc->set_client_id(typed.clientId);
        loc->mutable_position()->set_x(typed.position.x);
        loc->mutable_position()->set_y(typed.position.y);
        loc->mutable_position()->set_z(typed.position.z);
        loc->mutable_rotation()->set_x(typed.rotation.x);
        loc->mutable_rotation()->set_y(typed.rotation.y);
        loc->mutable_rotation()->set_z(typed.rotation.z);
        loc->mutable_rotation()->set_w(typed.rotation.w);
        loc->mutable_velocity()->set_x(typed.velocity.x);
        loc->mutable_velocity()->set_y(typed.velocity.y);
        loc->mutable_velocity()->set_z(typed.velocity.z);
        break;
    }
    case ServerMsg_Type_PLAYER_SPAWN: {
        msg.set_type(bz::ServerMsg::PLAYER_SPAWN);
        const auto &typed = static_cast<const ServerMsg_PlayerSpawn&>(input);
        auto* spawn = msg.mutable_player_spawn();
        spawn->set_client_id(typed.clientId);
        spawn->mutable_position()->set_x(typed.position.x);
        spawn->mutable_position()->set_y(typed.position.y);
        spawn->mutable_position()->set_z(typed.position.z);
        spawn->mutable_rotation()->set_x(typed.rotation.x);
        spawn->mutable_rotation()->set_y(typed.rotation.y);
        spawn->mutable_rotation()->set_z(typed.rotation.z);
        spawn->mutable_rotation()->set_w(typed.rotation.w);
        spawn->mutable_velocity()->set_x(typed.velocity.x);
        spawn->mutable_velocity()->set_y(typed.velocity.y);
        spawn->mutable_velocity()->set_z(typed.velocity.z);
        break;
    }
    case ServerMsg_Type_PLAYER_DEATH: {
        msg.set_type(bz::ServerMsg::PLAYER_DEATH);
        const auto &typed = static_cast<const ServerMsg_PlayerDeath&>(input);
        msg.mutable_player_death()->set_client_id(typed.clientId);
        break;
    }
    case ServerMsg_Type_CREATE_SHOT: {
        msg.set_type(bz::ServerMsg::CREATE_SHOT);
        const auto &typed = static_cast<const ServerMsg_CreateShot&>(input);
        auto* shot = msg.mutable_create_shot();
        shot->set_global_shot_id(typed.globalShotId);
        shot->mutable_position()->set_x(typed.position.x);
        shot->mutable_position()->set_y(typed.position.y);
        shot->mutable_position()->set_z(typed.position.z);
        shot->mutable_velocity()->set_x(typed.velocity.x);
        shot->mutable_velocity()->set_y(typed.velocity.y);
        shot->mutable_velocity()->set_z(typed.velocity.z);
        break;
    }
    case ServerMsg_Type_REMOVE_SHOT: {
        msg.set_type(bz::ServerMsg::REMOVE_SHOT);
        const auto &typed = static_cast<const ServerMsg_RemoveShot&>(input);
        auto* remove = msg.mutable_remove_shot();
        remove->set_shot_id(typed.shotId);
        remove->set_is_global_id(typed.isGlobalId);
        break;
    }
    case ServerMsg_Type_CHAT: {
        msg.set_type(bz::ServerMsg::CHAT);
        const auto &typed = static_cast<const ServerMsg_Chat&>(input);
        auto* chat = msg.mutable_chat();
        chat->set_from_id(typed.fromId);
        chat->set_to_id(typed.toId);
        chat->set_text(typed.text);
        break;
    }
    case ServerMsg_Type_INIT: {
        msg.set_type(bz::ServerMsg::INIT);
        const auto &typed = static_cast<const ServerMsg_Init&>(input);
        auto* init = msg.mutable_init();
        init->set_client_id(typed.clientId);
        init->set_server_name(typed.serverName);
        auto* params = init->mutable_default_player_params();
        for (const auto& [key, val] : typed.defaultPlayerParams) {
            (*params->mutable_params())[key] = val;
        }
        init->set_world_data(typed.worldData.data(), typed.worldData.size());
        break;
    }
    default:
        return std::nullopt;
    }

    std::string buffer;
    msg.SerializeToString(&buffer);
    return toBytes(buffer);
}

} // namespace net
