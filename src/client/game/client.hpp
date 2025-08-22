#pragma once
#include "geometry.hpp"
#include "timeutils.hpp"
#include "types.hpp"
#include "netmsg.hpp"
#include <threepp/threepp.hpp>
#include <string>
#include <ctime>
#include <chrono>
#include <vector>

class Client {
private:
    client_id clientId;
    std::string name;
    Location location;
    Location lastLocation;
    TimeUtils::time locationTime;
    TimeUtils::time lastLocationTime;
    bool alive = false;

    // bool IsClient(client_id clientId) const;
    // bool IsClient(std::string name) const;

    // std::string GetName() const;

    // Location GetLocation() const;
    
    // void SetLocation(Location location);

public:
    // static Client *GetClient(client_id clientId);

    Client(client_id clientId, std::string name);
    std::string ToString();

    template<ServerMsgSubType T> void Set(const T &msg) {
        if constexpr (std::is_same_v<T, ServerMsg_Location>) {
            location = msg.location;
        } else {
            static_assert(sizeof(T) == 0, "Client: Unsupported message type");
        }
    }

    std::string GetName() const {
        return name;
    }

    Location GetInterpolatedLocation() const;

    bool IsClient(client_id clientId) const {
        return this->clientId == clientId;
    }

    void SetLocation(Location location) {
        this->location = location;
    }

    void Spawn(Location location) {
        this->location = location;
        alive = true;
    }

    void Die() {
        alive = false;
    }

    bool IsAlive() const {
        return alive;
    }
};
