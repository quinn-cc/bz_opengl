#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>
#include <type_traits>
#include <cstddef>
#include <string>
#include <chrono>
#include <unordered_map>
#include <thread>

typedef struct InputState {
    bool fire;
    bool spawn;
    bool jump;
    bool quickQuit;
    bool chat;
    bool escape;
    glm::vec2 movement;
} InputState;

using SettingsMap = std::unordered_map<std::string, float>;
using ConfigMap = std::unordered_map<std::string, float>;

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

    inline void sleep(duration seconds) {
        std::this_thread::sleep_for(std::chrono::duration<float>(seconds));
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

#define THIS_CLIENT_ID 0
#define SERVER_CLIENT_ID 1
#define BROADCAST_CLIENT_ID 2
#define FIRST_CLIENT_ID 3

using client_id = uint32_t;
using shot_id = uint32_t;
using physics_id = uint32_t;
using render_id = uint32_t;
using audio_id = uint32_t;

#pragma pack(push, 1)

typedef struct PlayerState {
    glm::vec3 position;
    glm::vec3 rotation;
    bool alive;
    float speed;
    float jumpSpeed;
    float shotSpeed;
} PlayerState;

/*
 * Server messages
 */

enum ServerMsg_Type {
    ServerMsg_Type_PLAYER_JOIN,
    ServerMsg_Type_PLAYER_LEAVE,
    ServerMsg_Type_PLAYER_STATE,
    ServerMsg_Type_DEFAULT_PLAYER_STATE_CHANGE,
    ServerMsg_Type_PLAYER_LOCATION,
    ServerMsg_Type_PLAYER_SPAWN,
    ServerMsg_Type_PLAYER_DEATH,
    ServerMsg_Type_CREATE_SHOT,
    ServerMsg_Type_REMOVE_SHOT,
    ServerMsg_Type_INIT,
    ServerMsg_Type_CHAT
};

typedef struct ServerMsg {
    ServerMsg_Type type;
} ServerMsg;

typedef struct ServerMsg_PlayerJoin : ServerMsg {
    static constexpr ServerMsg_Type Type = ServerMsg_Type_PLAYER_JOIN;
    ServerMsg_PlayerJoin() { type = Type; }
    client_id clientId;
    std::string name;
    PlayerState state;
} ServerMsg_PlayerJoin;

typedef struct ServerMsg_PlayerLeave : ServerMsg {
    static constexpr ServerMsg_Type Type = ServerMsg_Type_PLAYER_LEAVE;
    ServerMsg_PlayerLeave() { type = Type; }
    client_id clientId;
} ServerMsg_PlayerLeave;

typedef struct ServerMsg_PlayerState : ServerMsg {
    static constexpr ServerMsg_Type Type = ServerMsg_Type_PLAYER_STATE;
    ServerMsg_PlayerState() { type = Type; }
    client_id clientId;
    PlayerState state;
} ServerMsg_PlayerState;

typedef struct ServerMsg_DefaultPlayerStateChange : ServerMsg {
    static constexpr ServerMsg_Type Type = ServerMsg_Type_DEFAULT_PLAYER_STATE_CHANGE;
    ServerMsg_DefaultPlayerStateChange() { type = Type; }
    client_id clientId;
    std::string key;
    float value;
} ServerMsg_DefaultPlayerStateChange;

typedef struct ServerMsg_PlayerLocation : ServerMsg {
    static constexpr ServerMsg_Type Type = ServerMsg_Type_PLAYER_LOCATION;
    ServerMsg_PlayerLocation() { type = Type; }
    client_id clientId;
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 velocity;
} ServerMsg_PlayerLocation;

typedef struct ServerMsg_PlayerSpawn : ServerMsg {
    static constexpr ServerMsg_Type Type = ServerMsg_Type_PLAYER_SPAWN;
    ServerMsg_PlayerSpawn() { type = Type; }
    client_id clientId;
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 velocity;
} ServerMsg_PlayerSpawn;

typedef struct ServerMsg_PlayerDeath : ServerMsg {
    static constexpr ServerMsg_Type Type = ServerMsg_Type_PLAYER_DEATH;
    ServerMsg_PlayerDeath() { type = Type; }
    client_id clientId;
} ServerMsg_PlayerDeath;
    
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

typedef struct ServerMsg_Chat : ServerMsg {
    static constexpr ServerMsg_Type Type = ServerMsg_Type_CHAT;
    ServerMsg_Chat() { type = Type; }
    client_id fromId;
    client_id toId;
    std::string text;
} ServerMsg_Chat;

typedef struct ServerMsg_Init : ServerMsg {
    static constexpr ServerMsg_Type Type = ServerMsg_Type_INIT;
    ServerMsg_Init() { type = Type; }
    client_id clientId;
    std::string serverName;
    std::byte* worldData;
} ServerMsg_Init;

/*
 * Client messages
 */

enum ClientMsg_Type {
    ClientMsg_Type_JOIN,
    ClientMsg_Type_LEAVE,
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

typedef struct ClientMsg_Join : ClientMsg {
    static constexpr ClientMsg_Type Type = ClientMsg_Type_JOIN;
    ClientMsg_Join() { type = Type; }
    std::string ip;
} ClientMsg_Join;

typedef struct ClientMsg_Leave : ClientMsg {
    static constexpr ClientMsg_Type Type = ClientMsg_Type_LEAVE;
    ClientMsg_Leave() { type = Type; }
} ClientMsg_Leave;

typedef struct ClientMsg_Init: ClientMsg {
    static constexpr ClientMsg_Type Type = ClientMsg_Type_INIT;
    ClientMsg_Init() { type = Type; }
    std::string name;
} ClientMsg_Init;

typedef struct ClientMsg_RequestSpawn : ClientMsg {
    static constexpr ClientMsg_Type Type = ClientMsg_Type_REQUEST_SPAWN;
    ClientMsg_RequestSpawn() { type = Type; }
} ClientMsg_RequestSpawn;

typedef struct ClientMsg_Location : ClientMsg {
    static constexpr ClientMsg_Type Type = ClientMsg_Type_LOCATION;
    ClientMsg_Location() { type = Type; }
    glm::vec3 position;
    glm::quat rotation;
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
    client_id toId;
    std::string text;
} ClientMsg_Chat;

#pragma pack(pop)