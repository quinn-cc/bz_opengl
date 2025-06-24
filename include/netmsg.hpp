#pragma once
#include "raylib.h"

#pragma pack(push, 1)

/*
 * Server messages
 */

enum ServerMsg_Type {
    ServerMsg_Type_LOCATION,
    ServerMsg_Type_DISCONNECTION,
    ServerMsg_Type_CONNECTION
};

typedef struct ServerMsg {
    ServerMsg_Type type;
} ServerMsg;

typedef struct ServerMsg_Location : ServerMsg {
    int clientId;
    Vector3 position;
    Quaternion rotation;
} ServerMsg_Location;

typedef struct ServerMsg_Connection : ServerMsg {
    int clientId;
    char name[256];
} ServerMsg_Connection;

typedef struct ServerMsg_Disconnection : ServerMsg {
    int clientId;
} ServerMsg_Disconnection;

/*
 * Client messages
 */

enum ClientMsg_Type {
    ClientMsg_Type_INIT,
    ClientMsg_Type_LOCATION
};

typedef struct ClientMsg {
    ClientMsg_Type type;
} ClientMsg;

typedef struct ClientMsg_Init : ClientMsg {
    char name[256];
} ClientMsg_Init;

typedef struct ClientMsg_Location : ClientMsg {
    Vector3 position;
    Quaternion rotation;
} ClientMsg_Location;

#pragma pack(pop)