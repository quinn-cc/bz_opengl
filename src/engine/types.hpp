#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>
#include <type_traits>
#include <cstddef>
#include <string>
#include <chrono>

typedef struct InputState {
    bool fire;
    bool spawn;
    bool jump;
    bool quickQuit;
    bool chat;
    bool escape;
    glm::vec2 movement;
} InputState;

namespace TimeUtils {
    using time = std::chrono::time_point<std::chrono::system_clock>;
    using duration = float;

    inline duration GetElapsedTime(time start, time end) {
        return std::chrono::duration<float>(end - start).count();
    }

    inline time GetCurrentTime() {
        return std::chrono::system_clock::now();
    }

    inline duration getDuration(float seconds) {
        return seconds;
    }
}

typedef struct Location {
    glm::vec3 position;
    glm::quat rotation;
} Location;

inline float angleBetween(const glm::quat& a, const glm::quat& b, bool degrees = true) {
    glm::quat qa = glm::normalize(a);
    glm::quat qb = glm::normalize(b);

    float d = glm::dot(qa, qb);
    d = glm::clamp(glm::abs(d), -1.0f, 1.0f);

    float ret = 2.0f * std::acos(d); // radians
    if (degrees) {
        ret = glm::degrees(ret);
    }
    return ret;
}

using client_id = uint32_t;
using shot_id = uint32_t;
using physics_id = uint32_t;
using render_id = uint32_t;

#pragma pack(push, 1)

/*
 * Server messages
 */

enum ServerMsg_Type {
    ServerMsg_Type_LOCATION,
    ServerMsg_Type_DISCONNECTION,
    ServerMsg_Type_CONNECTION,
    ServerMsg_Type_CREATE_SHOT,
    ServerMsg_Type_REMOVE_SHOT,
    ServerMsg_Type_ALLOW_SPAWN,
    ServerMsg_Type_SPAWN,
    ServerMsg_Type_DEATH,
    ServerMsg_Type_CHAT
};

typedef struct ServerMsg {
    ServerMsg_Type type;
} ServerMsg;

typedef struct ServerMsg_Location : ServerMsg {
    static constexpr ServerMsg_Type Type = ServerMsg_Type_LOCATION;
    ServerMsg_Location() { type = Type; }
    client_id clientId;
    Location location;
} ServerMsg_Location;

typedef struct ServerMsg_Connection : ServerMsg {
    static constexpr ServerMsg_Type Type = ServerMsg_Type_CONNECTION;
    ServerMsg_Connection() { type = Type; }
    client_id clientId;
    char name[256];
    Location location;
    bool alive;
} ServerMsg_Connection;

typedef struct ServerMsg_Disconnection : ServerMsg {
    static constexpr ServerMsg_Type Type = ServerMsg_Type_DISCONNECTION;
    ServerMsg_Disconnection() { type = Type; }
    client_id clientId;
} ServerMsg_Disconnection;

typedef struct ServerMsg_CreateShot : ServerMsg {
    static constexpr ServerMsg_Type Type = ServerMsg_Type_CREATE_SHOT;
    ServerMsg_CreateShot() { type = Type; }
    shot_id globalShotId;
    glm::vec3 position;
    glm::vec3 velocity;
} ServerMsg_CreateShot;

typedef struct ServerMsg_RemoveShot : ServerMsg {
    static constexpr ServerMsg_Type Type = ServerMsg_Type_REMOVE_SHOT;
    ServerMsg_RemoveShot() { type = Type; }
    shot_id shotId;
    bool isGlobalId;
} ServerMsg_RemoveShot;

typedef struct ServerMsg_AllowSpawn : ServerMsg {
    static constexpr ServerMsg_Type Type = ServerMsg_Type_ALLOW_SPAWN;
    ServerMsg_AllowSpawn() { type = Type; }
    bool allow;
    Location location;
} ServerMsg_AllowSpawn;

typedef struct ServerMsg_Spawn : ServerMsg {
    static constexpr ServerMsg_Type Type = ServerMsg_Type_SPAWN;
    ServerMsg_Spawn() { type = Type; }
    client_id clientId;
    Location location;
} ServerMsg_Spawn;

typedef struct ServerMsg_Death : ServerMsg {
    static constexpr ServerMsg_Type Type = ServerMsg_Type_DEATH;
    ServerMsg_Death() { type = Type; }
    client_id clientId;
} ServerMsg_Death;

typedef struct ServerMsg_Chat : ServerMsg {
    static constexpr ServerMsg_Type Type = ServerMsg_Type_CHAT;
    ServerMsg_Chat() { type = Type; }
    char name[256];
    char text[512];
} ServerMsg_Chat;


/*
 * Client messages
 */

enum ClientMsg_Type {
    ClientMsg_Type_CONNECTION,
    ClientMsg_Type_DISCONNECTION,
    ClientMsg_Type_INIT,
    ClientMsg_Type_REQUEST_SPAWN,
    ClientMsg_Type_LOCATION,
    ClientMsg_Type_CREATE_SHOT,
    ClientMsg_Type_CHAT
};

typedef struct ClientMsg {
    ClientMsg_Type type;
    client_id clientId;
} ClientMsg;

typedef struct ClientMsg_Connection : ClientMsg {
    static constexpr ClientMsg_Type Type = ClientMsg_Type_CONNECTION;
    ClientMsg_Connection() { type = Type; }
    char ip[64];
} ClientMsg_Connection;

typedef struct ClientMsg_Disconnection : ClientMsg {
    static constexpr ClientMsg_Type Type = ClientMsg_Type_DISCONNECTION;
    ClientMsg_Disconnection() { type = Type; }
} ClientMsg_Disconnection;

typedef struct ClientMsg_Init: ClientMsg {
    static constexpr ClientMsg_Type Type = ClientMsg_Type_INIT;
    ClientMsg_Init() { type = Type; }
    char name[256];
} ClientMsg_Init;

typedef struct ClientMsg_RequestSpawn : ClientMsg {
    static constexpr ClientMsg_Type Type = ClientMsg_Type_REQUEST_SPAWN;
    ClientMsg_RequestSpawn() { type = Type; }
} ClientMsg_RequestSpawn;

typedef struct ClientMsg_Location : ClientMsg {
    static constexpr ClientMsg_Type Type = ClientMsg_Type_LOCATION;
    ClientMsg_Location() { type = Type; }
    Location location;
} ClientMsg_Location;

typedef struct ClientMsg_CreateShot : ClientMsg {
    static constexpr ClientMsg_Type Type = ClientMsg_Type_CREATE_SHOT;
    ClientMsg_CreateShot() { type = Type; }
    shot_id localShotId;
    glm::vec3 position;
    glm::vec3 velocity;
} ClientMsg_CreateShot;

typedef struct ClientMsg_Chat : ClientMsg {
    static constexpr ClientMsg_Type Type = ClientMsg_Type_CHAT;
    ClientMsg_Chat() { type = Type; }
    char text[512];
} ClientMsg_Chat;

#pragma pack(pop)