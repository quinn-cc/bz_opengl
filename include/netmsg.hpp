#pragma once
#include <glm/glm.hpp>
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
    ServerMsg_Type_REMOVE_SHOT
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

/*
 * Client messages
 */

enum ClientMsg_Type {
    ClientMsg_Type_INIT,
    ClientMsg_Type_LOCATION,
    ClientMsg_Type_SHOT
};

typedef struct ClientMsg {
    ClientMsg_Type type;
} ClientMsg;

typedef struct ClientMsg_Init : ClientMsg {
    char name[256];
} ClientMsg_Init;

typedef struct ClientMsg_Location : ClientMsg {
    Location location;
} ClientMsg_Location;

typedef struct ClientMsg_Shot : ClientMsg {
    shot_id localShotId;
    glm::vec3 position;
    glm::vec3 velocity;
} ClientMsg_Shot;

#pragma pack(pop)