#pragma once
#include "client.hpp"

class Shot {
private:
    shot_id id;
    client_id ownerId;

public:
    Shot(shot_id id, client_id ownerId) : id(id), ownerId(ownerId) {}
    ~Shot() = default;

    void update();
};