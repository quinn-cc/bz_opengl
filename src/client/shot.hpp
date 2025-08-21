#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include "types.hpp"
#include "player.hpp"

#define BULLET_SIZE 1

class Shot {
private:
    glm::vec3 position;
    glm::vec3 velocity;
    client_id ownerId;
    shot_id id;

    static std::vector<std::function<void(Shot*)>> callbacks_add;
    static std::vector<std::function<void(Shot*)>> callbacks_remove;

public:
    static std::vector<Shot *> shots;
    static Shot* GetShotByGlobalId(shot_id globalId);
    static Shot* GetShotByLocalId(shot_id localId);
    static shot_id GenerateLocalShotId();
    static void AddCallback_AddShot(std::function<void(Shot*)> func);
    static void AddCallback_RemoveShot(std::function<void(Shot*)> func); 

    Shot(shot_id globalId, glm::vec3 position, glm::vec3 velocity);
    Shot(glm::vec3 position, glm::vec3 velocity) : Shot(GenerateLocalShotId(), position, velocity) {
        ownerId = 0;
    };
    ~Shot();
    void Update();
    glm::vec3 GetPosition();
    glm::vec3 GetVelocity();
    shot_id GetId() const;
};