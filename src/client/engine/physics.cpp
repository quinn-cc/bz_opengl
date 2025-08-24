#include "physics.hpp"
#include "geometry.hpp"
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include "bullet/btBulletCollisionCommon.h"
#include "game.hpp"
#include "world.hpp"

#define MOVE_SPEED 8
#define JUMP_VELOCITY 9

void Physics::Init(Game *game) {
    this->game = game;

    broadphase = new btDbvtBroadphase();
    collisionConfig = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfig);
    solver = new btSequentialImpulseConstraintSolver();
    world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);
    world->setGravity(btVector3(0, -9.8, 0));

    std::vector<MeshData> meshes = loadGLB("../data/world2.glb");

    for (auto &mesh : meshes) {
        // Create convex hull shape
        btConvexHullShape* convexShape = new btConvexHullShape();
        for (auto &v : mesh.vertices) {
            convexShape->addPoint(btVector3(v.x, v.y, v.z));
        }

        // Create motion state at origin
        btDefaultMotionState* motionState = new btDefaultMotionState(
            btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0))
        );

        // Static object → mass = 0
        btScalar mass = 0.0f;
        btVector3 inertia(0, 0, 0);
        if (mass > 0.0f)
            convexShape->calculateLocalInertia(mass, inertia);

        // Create rigid body
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, convexShape, inertia);
        btRigidBody* body = new btRigidBody(rbInfo);

        // Optional: physics properties
        body->setFriction(0.0f);
        body->setRestitution(0.0f);
        body->setRollingFriction(0.0f);
        body->setSpinningFriction(0.0f);

        // Add to world
        world->addRigidBody(body);
    }
    // floorShape = new btBoxShape(btVector3(50, 1, 50));
    // btTransform floorTransform;
    // floorTransform.setIdentity();
    // floorTransform.setOrigin(btVector3(0, -1, 0));
    // btRigidBody::btRigidBodyConstructionInfo floorInfo(
    //     0.0f, new btDefaultMotionState(floorTransform), floorShape, btVector3(0, 0, 0));
    // btRigidBody* floorBody = new btRigidBody(floorInfo);
    // world->addRigidBody(floorBody);

    // btBoxShape *boxShape = new btBoxShape(btVector3(2.5, 2.5, 2.5));
    // btTransform boxTransform;
    // boxTransform.setIdentity();
    // boxTransform.setOrigin(btVector3(10, 0, 10));
    // btRigidBody::btRigidBodyConstructionInfo boxInfo(
    //     0.0f, new btDefaultMotionState(boxTransform), boxShape, btVector3(0, 0, 0));
    // btRigidBody* boxBody = new btRigidBody(boxInfo);
    // world->addRigidBody(boxBody);

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
    playerBody->setRestitution(0.0f);    // bounciness
    playerBody->setRollingFriction(0.0f);
    playerBody->setSpinningFriction(0.0f);
    playerBody->setDamping(0, 0);
    playerBody->setAngularFactor(btVector3(0, .1, 0));
    playerBody->setLinearFactor(btVector3(1, 1, 1));

    world->addRigidBody(playerBody);
}

bool Physics::Player_IsGrounded() {
    btVector3 halfExtents(0.5f, 1.0f, 0.5f);  // adjust to match your box size
    btBoxShape boxShape(halfExtents);

    btTransform from = playerBody->getWorldTransform();
    btTransform to = from;
    to.setOrigin(from.getOrigin() - btVector3(0, 0.1f, 0));  // small downward distance

    // Setup callback
    btCollisionWorld::ClosestConvexResultCallback cb(from.getOrigin(), to.getOrigin());
    cb.m_collisionFilterGroup = playerBody->getBroadphaseHandle()->m_collisionFilterGroup;
    cb.m_collisionFilterMask  = playerBody->getBroadphaseHandle()->m_collisionFilterMask;

    // Sweep
    world->convexSweepTest(&boxShape, from, to, cb);

    bool isGrounded = cb.hasHit() && cb.m_hitNormalWorld.dot(btVector3(0,1,0)) > 0.7f;
    return isGrounded;
}

void Physics::Player_Move(glm::vec2 movement) {
    playerBody->activate(true);

    // btQuaternion yawRot;
    // yawRot.setEuler(-movement.x * Renderer::GetInstance().GetDeltaTime() * 2, 0, 0); // Euler: yaw, pitch, roll
    // btQuaternion newRot = yawRot * playerBody->getWorldTransform().getRotation();
    // playerBody->getWorldTransform().setRotation(newRot);
    
    btTransform trans1 = playerBody->getWorldTransform();
    btQuaternion rot = trans1.getRotation();
    btVector3 localVel(0, 0, movement.y * MOVE_SPEED); // local velocity
    btMatrix3x3 rotMat(rot);
    btVector3 worldVel = rotMat * localVel;
    btVector3 velocity = playerBody->getLinearVelocity();
    velocity.setX(worldVel.getX());
    velocity.setZ(worldVel.getZ());
    playerBody->setLinearVelocity(velocity);
    playerBody->setAngularVelocity(btVector3(0, -movement.x * 2, 0));
}

void Physics::Player_Jump() {
    playerBody->activate(true);
    btVector3 velocity = playerBody->getLinearVelocity();
    velocity.setY(JUMP_VELOCITY); // Set jump velocity (tune as needed)
    playerBody->setLinearVelocity(velocity);
}

void Physics::Player_SetLocation(Location location) {
    locationIsSet = true;
    locationSet = location;
    playerBody->setLinearVelocity(btVector3(0,0,0));
    playerBody->setAngularVelocity(btVector3(0,0,0));
    playerBody->clearForces();
    playerBody->activate();
}

Location Physics::Player_GetLocation() {
    Location location;
    btTransform trans = playerBody->getWorldTransform();
    location.position = glm::vec3(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ());
    location.rotation = glm::quat(trans.getRotation().getW(), trans.getRotation().getX(), trans.getRotation().getY(), trans.getRotation().getZ());
    return location;
}

glm::vec3 Physics::Player_GetVelocity() {
    btVector3 velocity = playerBody->getLinearVelocity();
    return glm::vec3(velocity.getX(), velocity.getY(), velocity.getZ());
}

void Physics::Update(float deltaTime) {
    if (locationIsSet) {
        playerBody->setWorldTransform(btTransform(
            btQuaternion(locationSet.rotation.x, locationSet.rotation.y, locationSet.rotation.z, locationSet.rotation.w),
            btVector3(locationSet.position.x, locationSet.position.y, locationSet.position.z)
        ));
        playerBody->setLinearVelocity(btVector3(0, 0, 0)); // clear velocity
        playerBody->setAngularVelocity(btVector3(0, 0, 0));
        locationIsSet = false;
    }

    for (Shot *shot : game->shots) {
        btVector3 rayStart = btVector3(shot->GetPosition().x, shot->GetPosition().y, shot->GetPosition().z);
        btVector3 rayEnd = btVector3(shot->GetAheadPosition().x, shot->GetAheadPosition().y, shot->GetAheadPosition().z);
        btCollisionWorld::ClosestRayResultCallback rayCallback(rayStart, rayEnd);
        world->rayTest(rayStart, rayEnd, rayCallback);

        if (rayCallback.hasHit()) {
            btVector3 hitNormal = rayCallback.m_hitNormalWorld;
            shot->RicochetAbout(glm::vec3(hitNormal.x(), hitNormal.y(), hitNormal.z()));
        }
    }

    // Step simulation
    world->stepSimulation(deltaTime, 10, 1.f/60.f);

    btTransform trans = playerBody->getWorldTransform();
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