#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "timeutils.hpp"
#include "types.hpp"

class Client;

class Shot {
private:
    TimeUtils::time startTime;
    glm::vec3 position;
    glm::vec3 velocity;
    Client *owner;
    shot_id localId;
    shot_id globalId;

public:
    static std::vector<Shot *> shots;
    static shot_id GenerateShotId();

    Shot(shot_id localId, Client *owner, glm::vec3 position, glm::vec3 velocity);
    ~Shot();
    void Update();
    shot_id GetLocalId() const;
    shot_id GetGlobalId() const;
    Client *GetOwner() const;
};