#pragma once
#include "bullet/btBulletDynamicsCommon.h"
#include "game.hpp"

class Physics {
private:
    Game *game;

    btBroadphaseInterface* broadphase;
    btDefaultCollisionConfiguration* collisionConfig;
    btCollisionDispatcher* dispatcher;
    btSequentialImpulseConstraintSolver* solver;
    btDiscreteDynamicsWorld* world;

    btCollisionShape* floorShape;
    btCollisionShape* playerShape;

    btRigidBody* playerBody;

    Location locationSet;
    bool locationIsSet = false;

public:
    bool Player_IsGrounded();
    void Player_Move(glm::vec2 movement);
    void Player_Jump();
    void Player_SetLocation(Location location);
    Location Player_GetLocation();
    glm::vec3 Player_GetVelocity();

    void Init(Game *game);
    void Update(float deltaTime);
    void Close();
};