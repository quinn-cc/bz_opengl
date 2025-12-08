#pragma once
#include "IRender.hpp"
#include "State.hpp"
#include <memory>

namespace Render {

    class Player : public IRender {
    private:
        const State::Player &state;
        std::shared_ptr<threepp::Mesh> radarMesh;

    public:
        Player(
            GLFWwindow &window,
            std::shared_ptr<threepp::Scene> scene,
            std::shared_ptr<threepp::Scene> radarScene,
            const State::Player &state
        ) : IRender(window, scene, radarScene), state(state) {
            /*
             * Scene
             */
            

            /*
             * Radar
             */
            auto geometry = threepp::CircleGeometry::create(0.7, 32); // radius 50 px
            auto material = threepp::MeshBasicMaterial::create();
            material->color = threepp::Color(0xffffff);
            material->transparent = false;
            material->depthTest = false; // always drawn on top
            radarMesh = threepp::Mesh::create(geometry, material);
            radarScene->add(radarMesh);
            radarMesh->rotation.x = -(glm::pi<float>()) / 2; // face up
        }

        ~Player() override {
            this->radarScene->remove(*radarMesh);
        }

        void update() override {
            glm::vec3 pos = state.location.position;
            radarMesh->position = toInternal(pos);
        }
    };

}