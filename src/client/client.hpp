#pragma once
#include <cstdint>
#include "engine/types.hpp"

class Client {
private:
    bool initialized;
    client_id id;

public:
    Client(client_id id);
    ~Client();

    void update();
};