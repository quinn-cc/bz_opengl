#pragma once
#include "engine/types.hpp"
#include <bullet/btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <string>
#include <map>
#include <vector>

#define NUM_SUBSTEPS (10)
#define FIXED_TIMESTEP (1.0f/60.0f)

class Game;

class Physics {
    friend class ServerEngine;
    friend class ClientEngine;

private:
    btBroadphaseInterface* broadphase;
    btDefaultCollisionConfiguration* collisionConfig;
    btCollisionDispatcher* dispatcher;
    btSequentialImpulseConstraintSolver* solver;
    btDiscreteDynamicsWorld* world;

    std::map<physics_id, std::vector<btRigidBody*>> bodies;

    Physics();
    ~Physics();

    std::vector<btRigidBody*> getBodies(physics_id id);
    physics_id getNextId();
    void update(float deltaTime);

public:
    physics_id createPlayer(glm::vec3 size);
    physics_id create(std::string meshPath, float mass);
    void destroy(physics_id id);

    void setGravity(float gravity);
    glm::vec3 getPosition(physics_id id);
    glm::quat getRotation(physics_id id);
    glm::vec3 getVelocity(physics_id id);
    glm::vec3 getAngularVelocity(physics_id id);
    glm::vec3 getForwardVector(physics_id id);
    void setPosition(physics_id id, const glm::vec3 &position);
    void setRotation(physics_id id, const glm::quat &rotation);
    void setVelocity(physics_id id, const glm::vec3 &velocity);
    void setAngularVelocity(physics_id id, const glm::vec3 &angularVelocity);
    bool isGrounded(physics_id id, glm::vec3 boxSize);
    bool raycast(const glm::vec3 &from, const glm::vec3 &to, glm::vec3 &hitPoint, glm::vec3 &hitNormal);
};