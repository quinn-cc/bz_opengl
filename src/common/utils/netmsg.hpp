#pragma once
#include <glm/glm.hpp>
#include <type_traits>
#include <cstddef>
#include <string>
#include "geometry.hpp"
#include "types.hpp"

#pragma pack(push, 1)

/*
 * Server messages
 */

enum ServerMsg_Type {
    ServerMsg_Type_LOCATION,
    ServerMsg_Type_DISCONNECTION,
    ServerMsg_Type_CONNECTION,
    ServerMsg_Type_SHOT,
    ServerMsg_Type_REMOVE_SHOT,
    ServerMsg_Type_ALLOW_SPAWN,
    ServerMsg_Type_SPAWN,
    ServerMsg_Type_DEATH,
    ServerMsg_Type_PLAYER_STATE
};

typedef struct ServerMsg {
    ServerMsg_Type type;
} ServerMsg;

typedef struct ServerMsg_Location : ServerMsg {
    client_id clientId;
    Location location;
} ServerMsg_Location;

typedef struct ServerMsg_Connection : ServerMsg {
    client_id clientId;
    char name[256];
} ServerMsg_Connection;

typedef struct ServerMsg_Disconnection : ServerMsg {
    client_id clientId;
} ServerMsg_Disconnection;

typedef struct ServerMsg_Shot : ServerMsg {
    shot_id globalShotId;
    glm::vec3 position;
    glm::vec3 velocity;
} ServerMsg_Shot;

typedef struct ServerMsg_RemoveShot : ServerMsg {
    client_id clientId;
    shot_id shotId;
} ServerMsg_RemoveShot;

typedef struct ServerMsg_AllowSpawn : ServerMsg {
    bool allow;
    Location location;
} ServerMsg_AllowSpawn;

typedef struct ServerMsg_Spawn : ServerMsg {
    client_id clientId;
    Location location;
} ServerMsg_Spawn;

typedef struct ServerMsg_Death : ServerMsg {
    client_id clientId;
} ServerMsg_Death;

typedef struct ServerMsg_PlayerState : ServerMsg {
    client_id clientId;
    char name[256];
    Location location;
    bool alive;
} ServerMsg_PlayerState;

/*
 * Client messages
 */

enum ClientMsg_Type {
    ClientMsg_Type_INIT,
    ClientMsg_Type_REQUEST_SPAWN,
    ClientMsg_Type_LOCATION,
    ClientMsg_Type_SHOT
};

typedef struct ClientMsg {
    ClientMsg_Type type;
} ClientMsg;

typedef struct ClientMsg_Init : ClientMsg {
    char name[256];
} ClientMsg_Init;

typedef struct ClientMsg_RequestSpawn : ClientMsg {
} ClientMsg_RequestSpawn;

typedef struct ClientMsg_Location : ClientMsg {
    Location location;
} ClientMsg_Location;

typedef struct ClientMsg_Shot : ClientMsg {
    shot_id localShotId;
    glm::vec3 position;
    glm::vec3 velocity;
} ClientMsg_Shot;

#pragma pack(pop)

template <typename T>
concept ServerMsgSubType = std::is_base_of_v<ServerMsg, T>;

template <typename V>
concept ClientMsgSubType = std::is_base_of_v<ClientMsg, V>;

inline std::string Debug_ClientMsgToString(const ClientMsg &msg) {
    switch (msg.type) {
    case ClientMsg_Type_INIT:
        return "ClientMsg_Init";
    case ClientMsg_Type_REQUEST_SPAWN:
        return "ClientMsg_RequestSpawn";
    case ClientMsg_Type_LOCATION:
        return "ClientMsg_Location";
    case ClientMsg_Type_SHOT:
        return "ClientMsg_Shot";
    default:
        return "Unknown ClientMsg";
    }
}

inline std::string Debug_ServerMsgToString(const ServerMsg &msg) {
    switch (msg.type) {
    case ServerMsg_Type_CONNECTION:
        return "ServerMsg_Connection";
    case ServerMsg_Type_DISCONNECTION:
        return "ServerMsg_Disconnection";
    case ServerMsg_Type_LOCATION:
        return "ServerMsg_Location";
    case ServerMsg_Type_SHOT:
        return "ServerMsg_Shot";
    case ServerMsg_Type_REMOVE_SHOT:
        return "ServerMsg_RemoveShot";
    case ServerMsg_Type_ALLOW_SPAWN:
        return "ServerMsg_AllowSpawn";
    case ServerMsg_Type_SPAWN:
        return "ServerMsg_Spawn";
    case ServerMsg_Type_DEATH:
        return "ServerMsg_Death";
    case ServerMsg_Type_PLAYER_STATE:
        return "ServerMsg_PlayerState";
    default:
        return "Unknown ServerMsg";
    }
}