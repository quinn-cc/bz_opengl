#pragma once
#include <string>
#include <cstring>
#include <vector>
#include "geometry.hpp"
#include "types.hpp"
#include "netmsg.hpp"

class Client {
public:
    client_id id;
    std::string name;
    Location location;
    bool alive;

    static client_id GenerateClientId() {
        static client_id id = 1;
        return id++;
    }

    Client() {
        this->id = GenerateClientId();
        this->alive = false;
    }

    std::string ToString();
};