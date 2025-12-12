#pragma once
#include <bullet/btBulletDynamicsCommon.h>
#include <glm/glm.hpp>



class Physics {
private:
    void update(float deltaTime);

    friend class Engine;

public:
    Physics();
    ~Physics();

    physics_id create(glm::vec3 size, float mass);
    void destroy(physics_id id);

    void setPosition(physics_id id, const glm::vec3 &position);
    void setVelocity(physics_id id, glm::vec3 velocity);
    void setRotation(physics_id id, const glm::quat &rotation);
    glm::vec3 getPosition(physics_id id);
    glm::quat getRotation(physics_id id);
    glm::vec3 getVelocity(physics_id id);
    glm::vec3 getAngularVelocity(physics_id id);
    bool isGrounded(physics_id id);

    void move(physics_id id, glm::vec3 movement);
};