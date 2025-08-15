#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <functional>

#define BULLET_SIZE 1

class Shot {
private:
    glm::vec3 position;
    glm::vec3 velocity;
    int ownerId;

    static std::vector<std::function<void(Shot*)>> callbacks_add;
    static std::vector<std::function<void(Shot*)>> callbacks_remove;

public:
    static std::vector<Shot *> shots;
    static void AddCallback_AddShot(std::function<void(Shot*)> func);
    static void AddCallback_RemoveShot(std::function<void(Shot*)> func);    

    Shot(int ownerId, glm::vec3 position, glm::vec3 velocity);
    ~Shot();
    void Update();
    glm::vec3 GetPosition();
    glm::vec3 GetVelocity();
};