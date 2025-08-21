#pragma once
#include "bullet/btBulletDynamicsCommon.h"

class Physics {
private:
    btBroadphaseInterface* broadphase;
    btDefaultCollisionConfiguration* collisionConfig;
    btCollisionDispatcher* dispatcher;
    btSequentialImpulseConstraintSolver* solver;
    btDiscreteDynamicsWorld* world;

    btCollisionShape* floorShape;
    btCollisionShape* playerShape;

    btRigidBody* playerBody;

public:
    static Physics &GetInstance();

    void Init();
    bool IsGrounded();
    void Update();
    void Close();
};