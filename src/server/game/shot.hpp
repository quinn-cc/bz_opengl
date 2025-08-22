#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "timeutils.hpp"
#include "types.hpp"
#include "client.hpp"

#define BULLET_SIZE 1
#define SHOT_LIFETIME 1.0f

class Shot {
private:
    TimeUtils::time startTime;
    

public:
    glm::vec3 position;
    glm::vec3 velocity;
    Client *owner;
    shot_id localId;
    shot_id globalId;

    static shot_id GenerateShotId() {
        static shot_id id = 1;
        return id++;
    }

    Shot(shot_id localId, Client *owner, glm::vec3 position, glm::vec3 velocity);
    void Update(TimeUtils::duration deltaTime);
    bool IsExpired() const;
};