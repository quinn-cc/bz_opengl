#pragma once
#include "bullet/btBulletCollisionCommon.h"
#include <vector>
#include "MeshLoader.hpp"
#include "IPhysics.hpp"

namespace Physics {

    class World : public IPhysics {
    public:
        World(btDiscreteDynamicsWorld &world) : IPhysics(world) {
            std::vector<MeshLoader::MeshData> meshes = MeshLoader::loadGLB("../data/world2.glb");

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
        }

        ~World() {
            
        }

        void update() {
            
        }
    };

}