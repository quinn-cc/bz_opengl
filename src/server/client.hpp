#pragma once
#include <string>
#include "engine/types.hpp"

class Client {
private:
    std::string ip;
    std::string name;
    client_id id;

public:
    Client(client_id id, std::string ip);
    ~Client() = default;

    bool isClient(client_id cid) const;
};