#pragma once
#include "State.hpp"
#include "IPhysics.hpp"
#include "bullet/btBulletDynamicsCommon.h"
#include <glm/glm.hpp>

namespace Physics {

    class Shot : public IPhysics {
    private:
        const State::Shot &shot;

    public:
        Shot(btDiscreteDynamicsWorld &world, const State::Shot &shot)
            : IPhysics(world), shot(shot) {
            // Initialize bullet physics objects here
        }

        void update() override {
            btVector3 rayStart = btVector3(shot.position.x, shot.position.y, shot.position.z);
            btVector3 rayEnd = btVector3(shot.GetAheadPosition().x, shot.GetAheadPosition().y, shot.GetAheadPosition().z);
            btCollisionWorld::ClosestRayResultCallback rayCallback(rayStart, rayEnd);
            world.rayTest(rayStart, rayEnd, rayCallback);

            if (rayCallback.hasHit()) {
                btVector3 hitNormal = rayCallback.m_hitNormalWorld;
                shot.RicochetAbout(glm::vec3(hitNormal.x(), hitNormal.y(), hitNormal.z()));
            }
        }

        glm::vec3 getPosition() const {
            return shot.position;
        }
    };

}