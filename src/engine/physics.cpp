#include "engine/physics.hpp"
#include "engine/types.hpp"
#include "engine/mesh_loader.hpp"
#include "spdlog/spdlog.h"

Physics::Physics() {
    broadphase = new btDbvtBroadphase();
    collisionConfig = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfig);
    solver = new btSequentialImpulseConstraintSolver();
    world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);
    world->setGravity(btVector3(0, -9.8f, 0));
}

Physics::~Physics() {
    delete world;
    delete solver;
    delete dispatcher;
    delete collisionConfig;
    delete broadphase;
}

std::vector<btRigidBody*> Physics::getBodies(physics_id id) {
    auto it = bodies.find(id);
    if (it != bodies.end()) {
        return it->second;
    }
    spdlog::error("Physics::getBodies: Invalid physics_id {}", id);
    return std::vector<btRigidBody*>();
}

physics_id Physics::getNextId() {
    static physics_id nextId = 1;

    while (nextId == 0 || bodies.find(nextId) != bodies.end()) {
        nextId++;
    }

    return nextId++;
}

void Physics::update(float deltaTime) {
    world->stepSimulation(deltaTime, NUM_SUBSTEPS, FIXED_TIMESTEP);
}

physics_id Physics::create(std::string meshPath, float mass) {
    physics_id id = getNextId();
    std::vector<btRigidBody*> bodyList;
    std::vector<MeshLoader::MeshData> meshes = MeshLoader::loadGLB(meshPath);

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

        // Static object mass = 0
        btScalar bMass = mass;
        btVector3 inertia(0, 0, 0);
        if (bMass > 0.0f)
            convexShape->calculateLocalInertia(bMass, inertia);

        // Create rigid body
        btRigidBody::btRigidBodyConstructionInfo rbInfo(bMass, motionState, convexShape, inertia);
        btRigidBody* body = new btRigidBody(rbInfo);

        // Optional: physics properties
        body->setFriction(0.0f);
        body->setRestitution(0.0f);
        body->setRollingFriction(0.0f);
        body->setSpinningFriction(0.0f);

        // Add to world
        world->addRigidBody(body);
        bodyList.push_back(body);
    }

    bodies[id] = bodyList;
    return id;
}

physics_id Physics::createPlayer(glm::vec3 size, float mass) {
    // Player (dynamic box)
    btBoxShape *playerShape = new btBoxShape(btVector3(size.x / 2, size.y / 2, size.z / 2));
    btTransform playerTransform;
    playerTransform.setIdentity();
    playerTransform.setOrigin(btVector3(0, 2, 0));

    btScalar bMass = 1.0f;
    btVector3 inertia(0, 0, 0);
    playerShape->calculateLocalInertia(bMass, inertia);

    btDefaultMotionState* motionState = new btDefaultMotionState(playerTransform);
    btRigidBody::btRigidBodyConstructionInfo playerInfo(mass, motionState, playerShape, inertia);
    btRigidBody* playerBody = new btRigidBody(playerInfo);

    // Disable inertia/rotation for arcade-style instant movement
    playerBody->setFriction(0);
    playerBody->setRestitution(0.0f);    // bounciness
    playerBody->setRollingFriction(0.0f);
    playerBody->setSpinningFriction(0.0f);
    playerBody->setDamping(0, 0);
    playerBody->setAngularFactor(btVector3(0, .1, 0));
    playerBody->setLinearFactor(btVector3(1, 1, 1));

    world->addRigidBody(playerBody);

    std::vector<btRigidBody*> bodyList;
    bodyList.push_back(playerBody);
    bodies[0] = bodyList;

    return 0;
}

void Physics::destroy(physics_id id) {
    auto it = bodies.find(id);
    if (it != bodies.end()) {
        for (btRigidBody* body : it->second) {
            world->removeRigidBody(body);
            delete body->getMotionState();
            delete body->getCollisionShape();
            delete body;
        }
        bodies.erase(it);
    }
}

void Physics::setGravity(float gravity) {
    world->setGravity(btVector3(0, gravity, 0));
}

glm::vec3 Physics::getPosition(physics_id id) {
    // Just return position of first body
    auto bodiesList = getBodies(id);
    if (bodiesList.size() == 1) { spdlog::error("This function only works for single-body physics_ids"); }
    btRigidBody* body = bodiesList.front();
    btTransform transform;
    body->getMotionState()->getWorldTransform(transform);
    btVector3 pos = transform.getOrigin();
    return glm::vec3(pos.x(), pos.y(), pos.z());
}

glm::quat Physics::getRotation(physics_id id) {
    // Just return rotation of first body
    auto bodiesList = getBodies(id);
    if (bodiesList.size() == 1) { spdlog::error("This function only works for single-body physics_ids"); }
    btRigidBody* body = bodiesList.front();
    btTransform transform;
    body->getMotionState()->getWorldTransform(transform);
    btQuaternion rot = transform.getRotation();
    return glm::quat(rot.w(), rot.x(), rot.y(), rot.z());
}

glm::vec3 Physics::getVelocity(physics_id id) {
    // Just return velocity of first body
    auto bodiesList = getBodies(id);
    if (bodiesList.size() == 1) { spdlog::error("This function only works for single-body physics_ids"); }
    btRigidBody* body = bodiesList.front();
    btVector3 vel = body->getLinearVelocity();
    return glm::vec3(vel.x(), vel.y(), vel.z());
}

glm::vec3 Physics::getAngularVelocity(physics_id id) {
    // Just return angular velocity of first body
    auto bodiesList = getBodies(id);
    if (bodiesList.size() == 1) { spdlog::error("This function only works for single-body physics_ids"); }
    btRigidBody* body = bodiesList.front();
    btVector3 vel = body->getAngularVelocity();
    return glm::vec3(vel.x(), vel.y(), vel.z());
}

void Physics::setPosition(physics_id id, const glm::vec3 &position) {
    auto bodiesList = getBodies(id);
    if (bodiesList.size() == 1) { spdlog::error("This function only works for single-body physics_ids"); }
    btRigidBody* body = bodiesList.front();
    btTransform transform;
    body->getMotionState()->getWorldTransform(transform);
    transform.setOrigin(btVector3(position.x, position.y, position.z));
    body->getMotionState()->setWorldTransform(transform);
    body->setWorldTransform(transform);
    body->activate();
}

void Physics::setRotation(physics_id id, const glm::quat &rotation) {
    auto bodiesList = getBodies(id);
    if (bodiesList.size() == 1) { spdlog::error("This function only works for single-body physics_ids"); }
    btRigidBody* body = bodiesList.front();
    btTransform transform;
    body->getMotionState()->getWorldTransform(transform);
    transform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
    body->getMotionState()->setWorldTransform(transform);
    body->setWorldTransform(transform);
    body->activate();
}

void Physics::setVelocity(physics_id id, const glm::vec3 &velocity) {
    auto bodiesList = getBodies(id);
    if (bodiesList.size() == 1) { spdlog::error("This function only works for single-body physics_ids"); }
    btRigidBody* body = bodiesList.front();
    body->setLinearVelocity(btVector3(velocity.x, velocity.y, velocity.z));
    body->activate();
}

void Physics::setAngularVelocity(physics_id id, const glm::vec3 &angularVelocity) {
    auto bodiesList = getBodies(id);
    if (bodiesList.size() == 1) { spdlog::error("This function only works for single-body physics_ids"); }
    btRigidBody* body = bodiesList.front();
    body->setAngularVelocity(btVector3(angularVelocity.x, angularVelocity.y, angularVelocity.z));
    body->activate();
}