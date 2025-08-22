#pragma once
#include <glm/glm.hpp>
#include <string>
#include <cstring>
#include "geometry.hpp"
#include "netmsg.hpp"

class Player {
private:
    Location location;
    Location lastLocation;
    std::string name;
    float moveSpeed;
    float turnSpeed;
    bool updateLoc;
    bool alive;
    bool canSpawn;
    

public:
    static Player &GetInstance();
    
    void Init(std::string name);
    void Update();

    void SetLocation(Location location) {
        this->location = location;
    }

    Location GetLocation() const {
        return location;
    }

    glm::vec3 GetForwardVector() {
        return location.rotation * glm::vec3(0, 0, 1);
    }

    std::string GetName() const {
        return name;
    }

    bool IsAlive() const {
        return alive;
    }

    bool CanSpawn() const {
        return canSpawn;
    }

    void SetCanSpawn(bool canSpawn) {
        this->canSpawn = canSpawn;
    }

    void Spawn(Location location) {
        this->location = location;
        alive = true;
        canSpawn = false;
    }

    void Die() {
        alive = false;
        canSpawn = false;
    }

    void Close();
    bool LocationChanged();

    template<ServerMsgSubType T> void Set(const T &msg) {
        if constexpr (std::is_same_v<T, ServerMsg_Location>) {
            location = msg.location;
        } else {
            static_assert(sizeof(T) == 0, "Client: Unsupported message type");
        }
    }

    template<ClientMsgSubType T> T Get() {
        if constexpr (std::is_same_v<T, ClientMsg_Location>) {
            ClientMsg_Location msg;
            msg.type = ClientMsg_Type_LOCATION;
            msg.location = location;
            return msg;
        } else if constexpr (std::is_same_v<T, ClientMsg_Init>) {
            ClientMsg_Init msg;
            msg.type = ClientMsg_Type_INIT;
            strncpy(msg.name, name.c_str(), name.size());
            msg.name[name.size()] = '\0';
            return msg;
        } else if constexpr (std::is_same_v<T, ClientMsg_RequestSpawn>) {
            ClientMsg_RequestSpawn msg;
            msg.type = ClientMsg_Type_REQUEST_SPAWN;
            return msg;
        } else if constexpr (std::is_same_v<T, ClientMsg_Spawn>) {
            ClientMsg_Spawn msg;
            msg.type = ClientMsg_Type_SPAWN;
            return msg;
        } else {
            static_assert(sizeof(T) == 0, "Client: Unsupported message type");
        }
    }
};