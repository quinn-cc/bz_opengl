#pragma once
#include <glm/glm.hpp>
#include "geometry.hpp"

#pragma pack(push, 1)

/*
 * Server messages
 */

enum ServerMsg_Type {
    ServerMsg_Type_LOCATION,
    ServerMsg_Type_DISCONNECTION,
    ServerMsg_Type_CONNECTION,
    ServerMsg_Type_SHOT
};

typedef struct ServerMsg {
    ServerMsg_Type type;
} ServerMsg;

typedef struct ServerMsg_Location : ServerMsg {
    int clientId;
    Location location;
} ServerMsg_Location;

typedef struct ServerMsg_Connection : ServerMsg {
    int clientId;
    char name[256];
} ServerMsg_Connection;

typedef struct ServerMsg_Disconnection : ServerMsg {
    int clientId;
} ServerMsg_Disconnection;

typedef struct ServerMsg_Shot : ServerMsg {
    int clientId;
    glm::vec3 position;
    glm::vec3 velocity;
} ServerMsg_Shot;

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
    glm::vec3 position;
    glm::vec3 velocity;
} ClientMsg_Shot;

#pragma pack(pop)