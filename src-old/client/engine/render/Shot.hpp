#pragma once
#include "IRender.hpp"
#include "State.hpp"
#include <memory>

#define RADAR_SHOT_SIZE 0.25f

namespace Render {

    class Shot : public IRender {
    private:
        const State::Shot &state;
        std::shared_ptr<threepp::Mesh> mesh;
        std::shared_ptr<threepp::Mesh> radarMesh;

    public:
        Shot(
            GLFWwindow &window,
            std::shared_ptr<threepp::Scene> scene,
            std::shared_ptr<threepp::Scene> radarScene,
            const State::Shot &state
        ) : IRender(window, scene, radarScene), state(state) {
            /*
             * Scene
             */
            auto geometry = threepp::SphereGeometry::create(1);
            static auto material = threepp::MeshStandardMaterial::create();
            material->color = threepp::Color(0x00ffff); // green
            material->metalness = 0.5f;   // optional: 0 = non-metal, 1 = metal
            material->roughness = 0.5f;   // 0 = smooth, 1 = rough
            std::shared_ptr<threepp::Mesh> mesh = threepp::Mesh::create(geometry, material);
            mesh->castShadow = true;
            mesh->receiveShadow = true;
            this->mesh = mesh;
            this->scene->add(mesh);

            /*
             * Radar
             */
            std::shared_ptr<threepp::MeshBasicMaterial> radarShotMaterial;
            radarShotMaterial = threepp::MeshBasicMaterial::create();
            radarShotMaterial->color = threepp::Color(0xffffff);
            radarShotMaterial->transparent = false;
            radarShotMaterial->depthTest = false;
            auto radarGeometry = threepp::CircleGeometry::create(RADAR_SHOT_SIZE, 32);
            auto radarShot = threepp::Mesh::create(radarGeometry, radarShotMaterial);
            radarShot->rotation.x = -(glm::pi<float>()) / 2;
            radarScene->add(radarShot);
        }

        ~Shot() override {
            this->scene->remove(*mesh);
        }

        void update() override {
            glm::vec3 pos = state.position;
            mesh->position = toInternal(pos);
            radarMesh->position = toInternal(pos);
        }
    };

}