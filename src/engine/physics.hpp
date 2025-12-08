#pragma once
#include "bullet/btBulletDynamicsCommon.h"
#include <glm/glm.hpp>

typedef unsigned int render_id;

class Physics {
private:
    void update(float deltaTime);

    friend class Engine;

public:
    Physics();
    ~Physics();

    render_id create(glm::vec3 size, float mass);
    void destroy(render_id id);

    void setPosition(render_id id, const glm::vec3 &position);
    void setVelocity(render_id id, glm::vec3 velocity);
    void setRotation(render_id id, const glm::quat &rotation);
    glm::vec3 getPosition(render_id id);
    glm::quat getRotation(render_id id);
    glm::vec3 getVelocity(render_id id);
    glm::vec3 getAngularVelocity(render_id id);
    bool isGrounded(render_id id);

    void move(render_id id, glm::vec3 movement);
};