#pragma once
#include "bullet/btBulletCollisionCommon.h"
#include <glm/glm.hpp>
#include "types.hpp"
#include "geometry.hpp"
#include "IPhysics.hpp"

namespace Physics {

    class Player : public IPhysics {
    private:
        btRigidBody* playerBody;
        btCollisionShape* playerShape;
        Location locationSet;
        bool locationIsSet = false;

    public:
        Player(btDiscreteDynamicsWorld &world) : IPhysics(world) {
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

        ~Player() {
            // Cleanup
            delete playerBody->getMotionState();
            delete playerBody;
            delete playerShape;
        }

        void update() {
            if (locationIsSet) {
                playerBody->setWorldTransform(btTransform(
                    btQuaternion(locationSet.rotation.x, locationSet.rotation.y, locationSet.rotation.z, locationSet.rotation.w),
                    btVector3(locationSet.position.x, locationSet.position.y, locationSet.position.z)
                ));
                playerBody->setLinearVelocity(btVector3(0, 0, 0)); // clear velocity
                playerBody->setAngularVelocity(btVector3(0, 0, 0));
                locationIsSet = false;
            }
        }

        void setLocation(Location location) {
            locationIsSet = true;
            locationSet = location;
            playerBody->setLinearVelocity(btVector3(0,0,0));
            playerBody->setAngularVelocity(btVector3(0,0,0));
            playerBody->clearForces();
            playerBody->activate();
        }

        Location getLocation() {
            Location location;
            btTransform transform;
            playerBody->getWorldTransform(transform);
            location.position = { transform.getOrigin().x(), transform.getOrigin().y(), transform.getOrigin().z() };
            location.rotation = { transform.getRotation().x(), transform.getRotation().y(), transform.getRotation().z(), transform.getRotation().w() };
            return location;
        }

        glm::vec3 getVelocity() {
            btVector3 velocity = playerBody->getLinearVelocity();
            return glm::vec3(velocity.getX(), velocity.getY(), velocity.getZ());
        }

        glm::vec3 move(glm::vec2 movement) {
            playerBody->activate(true);
            btTransform trans1 = playerBody->getWorldTransform();
            btQuaternion rot = trans1.getRotation();
            btVector3 localVel(0, 0, movement.y * MOVE_SPEED); // local velocity
            btMatrix3x3 rotMat(rot);
            btVector3 worldVel = rotMat * localVel;
            btVector3 velocity = playerBody->getLinearVelocity();
            velocity.setX(worldVel.getX());
            velocity.setZ(worldVel.getZ());
            playerBody->setLinearVelocity(velocity);
            playerBody->setAngularVelocity(btVector3(0, -movement.x * TURN_SPEED, 0));
        }

        void jump() {
            playerBody->activate(true);
            btVector3 velocity = playerBody->getLinearVelocity();
            velocity.setY(JUMP_VELOCITY); // Set jump velocity (tune as needed)
            playerBody->setLinearVelocity(velocity);
        }

        bool isGrounded() {
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
    };

}