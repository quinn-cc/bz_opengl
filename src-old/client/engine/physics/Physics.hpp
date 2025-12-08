#pragma once
#include "Player.hpp"
#include "World.hpp"
#include "bullet/btBulletDynamicsCommon.h"
#include "State.hpp"
#include <map>

namespace Physics {

    class Physics {
    private:

    public:
        btDbvtBroadphase broadphase;
        btDefaultCollisionConfiguration collisionConfig;
        btCollisionDispatcher dispatcher;
        btSequentialImpulseConstraintSolver solver;
        btDiscreteDynamicsWorld btWorld;

        Player player;
        World world;
        std::map<shot_id, std::shared_ptr<Shot>> shots;
        std::vector<std::shared_ptr<IPhysics>> physicsObjects;

        State::Game &gameState;

        Physics(State::Game &gameState)
            : broadphase(), collisionConfig(),
              dispatcher(&collisionConfig), solver(),
              btWorld(&dispatcher, &broadphase, &solver, &collisionConfig),
              player(btWorld), world(btWorld), gameState(gameState) {
            btWorld.setGravity(btVector3(0, -9.8, 0));

            physicsObjects.push_back(std::make_shared<Player>(player));
            physicsObjects.push_back(std::make_shared<World>(world));
        }

        void update() {
            for (auto &obj : physicsObjects) {
                obj->update();
            }

            btWorld.stepSimulation(gameState.deltaTime, 10, 1.f/60.f);
        }

        void addShot(const State::Shot &shot) {
            auto shotPtr = std::make_shared<State::Shot>(shot);
            shots.emplace(
                shot.id,
                shotPtr
            );
            physicsObjects.push_back(shotPtr);
        }

        void removeShot(const State::Shot &shot) {
            auto shotPtr = shots.at(shot.id);
            physicsObjects.erase(std::remove(physicsObjects.begin(), physicsObjects.end(), shotPtr), physicsObjects.end());
            shots.erase(shot.id);
        }
    };

}