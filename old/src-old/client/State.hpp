#pragma once
#include <glm/glm.hpp>
#include "types.hpp"
#include <string>
#include "geometry.hpp"
#include "timeutils.hpp"

namespace State {
    
    struct Shot {
        glm::vec3 position;
        glm::vec3 velocity;
        client_id ownerId;
        shot_id id;
    };

    struct User {
        std::string name;
        bool alive;
        Location location;
        Location lastLocation;

        glm::vec3 getForwardVector() const {
            return glm::normalize(location.rotation * glm::vec3(0, 0, 1));
        }
    };

    struct Player : User {
        glm::vec3 velocity;
        float moveSpeed;
        float turnSpeed;
        bool updateLoc;
        bool canSpawn;
    };

    struct Client : User {
        client_id id;
        TimeUtils::time locationTime;
        TimeUtils::time lastLocationTime;
    };

    struct Game {
        std::shared_ptr<Player> player;
        std::vector<std::shared_ptr<User>> users;
        TimeUtils::duration deltaTime;
    };
}