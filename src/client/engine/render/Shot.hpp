#pragma once
#include <IRender.hpp>
#include "State.hpp"
#include <memory>

namespace Render {

    class Shot : public IRender {
    private:
        const State::Shot &state;
        std::shared_ptr<threepp::Mesh> mesh;
        std::shared_ptr<threepp::Scene> scene;

    public:
        Shot(
            GLFWwindow &window,
            const State::Shot &state,
            std::shared_ptr<threepp::Scene> scene
        ) : IRender(window), state(state), scene(scene) {
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
        }

        ~Shot() override {
            this->scene->remove(*mesh);
        }

        void update() override {
            glm::vec3 pos = state.position;
            mesh->position = toInternal(pos);
        }
    };

}