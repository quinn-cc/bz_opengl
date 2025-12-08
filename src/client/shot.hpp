#pragma once
#include "client/client.hpp"

using shot_id = uint32_t;

class Shot {
private:
    shot_id id;
    client_id ownerId;

public:
    Shot(shot_id id, client_id ownerId) : id(id), ownerId(ownerId) {}
    ~Shot() = default;

    void update();
};