#pragma once
#include "bullet/btBulletCollisionCommon.h"

namespace Physics {
    
    class IPhysics {
    public:
        IPhysics(
            btDiscreteDynamicsWorld &world
        ) : IPhysics(world) {}
        virtual ~IPhysics() = default;
        virtual void update() = 0;
    };

}