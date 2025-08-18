#include "physics.hpp"
#include "input.hpp"
#include "player.hpp"
#include <spdlog/spdlog.h>
#include "btBulletCollisionCommon.h"

Physics &Physics::GetInstance() {
    static Physics instance;
    return instance;
}

void Physics::Init() {
    broadphase = new btDbvtBroadphase();
    collisionConfig = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfig);
    solver = new btSequentialImpulseConstraintSolver();
    world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);
    world->setGravity(btVector3(0, -50, 0));

    floorShape = new btBoxShape(btVector3(50, 1, 50));
    btTransform floorTransform;
    floorTransform.setIdentity();
    floorTransform.setOrigin(btVector3(0, -1, 0));
    btRigidBody::btRigidBodyConstructionInfo floorInfo(
        0.0f, new btDefaultMotionState(floorTransform), floorShape, btVector3(0, 0, 0));
    btRigidBody* floorBody = new btRigidBody(floorInfo);
    world->addRigidBody(floorBody);

    // Player (dynamic box)
    playerShape = new btBoxShape(btVector3(0.5, 1.0, 0.5));
    btTransform playerTransform;
    playerTransform.setIdentity();
    playerTransform.setOrigin(btVector3(0, 2, 0));

    btScalar mass = 1.0f;
    btVector3 inertia(0, 0, 0);
    playerShape->calculateLocalInertia(mass, inertia);

    btDefaultMotionState* motionState = new btDefaultMotionState(playerTransform);
    btRigidBody::btRigidBodyConstructionInfo playerInfo(mass, motionState, playerShape, inertia);
    playerBody = new btRigidBody(playerInfo);

    // Disable inertia/rotation for arcade-style instant movement
    playerBody->setFriction(0);
    playerBody->setDamping(0, 0);
    playerBody->setAngularFactor(btVector3(0, 1, 0));
    playerBody->setLinearFactor(btVector3(1, 1, 1));

    world->addRigidBody(playerBody);
}

void Physics::Update() {
    spdlog::debug("About to move");

    glm::vec2 movement = Input::GetInstance().GetMovement();
    playerBody->activate(true);

    playerBody->setAngularVelocity(btVector3(0, -movement.x * 2, 0));
    btTransform trans1;
    playerBody->getMotionState()->getWorldTransform(trans1);
    btQuaternion rot = trans1.getRotation();
    btVector3 localVel(0, 0, movement.y * 10); // local velocity
    btMatrix3x3 rotMat(rot);
    btVector3 worldVel = rotMat * localVel;
    playerBody->setLinearVelocity(worldVel);

    // Step simulation
    spdlog::debug("About to step");
    world->stepSimulation(1.f / 60.f);
    spdlog::debug("Stepped");

    Location location;
    btTransform trans;
    playerBody->getMotionState()->getWorldTransform(trans);
    location.position = glm::vec3(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ());
    location.rotation = glm::quat(trans.getRotation().getW(), trans.getRotation().getX(), trans.getRotation().getY(), trans.getRotation().getZ());
    Player::GetInstance().SetLocation(location);

    spdlog::debug("location x={},y={},z={}", location.position.x, location.position.y, location.position.z);
}

void Physics::Close() {
    delete world;
    delete solver;
    delete dispatcher;
    delete collisionConfig;
    delete broadphase;
    delete floorShape;
    delete playerShape;
}