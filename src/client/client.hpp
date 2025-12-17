#pragma once
#include <cstdint>
#include "engine/types.hpp"

class Client {
private:
    client_id id;

public:
    Client(client_id id) : id(id) {}
    ~Client() = default;

    void update();
};