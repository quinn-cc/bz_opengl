#pragma once
#include <cstdint>

using client_id = uint32_t;

class Client {
private:
    client_id id;

public:
    Client(client_id id) : id(id) {}
    ~Client() = default;

    void update();
};