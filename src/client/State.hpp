#pragma once
#include <glm/glm.hpp>
#include "types.hpp"
#include <string>
#include "geometry.hpp"

namespace State {
    struct Shot {
        glm::vec3 position;
        glm::vec3 velocity;
        client_id ownerId;
        shot_id id;
    };

    struct Player {
        Location location;
        Location lastLocation;
        glm::vec3 velocity;
        std::string name;
        float moveSpeed;
        float turnSpeed;
        bool updateLoc;
        bool alive;
        bool canSpawn;
    };
}