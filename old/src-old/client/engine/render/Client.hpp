#pragma once
#include "engine/render/IRender.hpp"
#include "State.hpp"
#include <memory>
#include <threepp/loaders/AssimpLoader.hpp>
#include <spdlog/spdlog.h>

#define RADAR_CLIENT_SIZE 0.7f

namespace Render {

    class Client : public IRender {
    private:
        const State::Client &state;
        std::shared_ptr<threepp::Object3D> mesh;
        std::shared_ptr<threepp::Mesh> radarMesh;

    public:
        Client(
            GLFWwindow &window,
            std::shared_ptr<threepp::Scene> scene,
            std::shared_ptr<threepp::Scene> radarScene,
            const State::Client &state
        ) : IRender(window, scene, radarScene), state(state) {
            /*
             * Scene
             */
            threepp::AssimpLoader loader;

            try {
                auto model = loader.load("../data/models/tank/tank.glb");
                mesh = model;
                model->scale.set(0.5f, 0.5f, 0.5f);
                model->traverseType<threepp::Mesh>([&](threepp::Mesh& child) {
                    child.castShadow = true;
                    child.receiveShadow = true;
                });
                scene->add(model);
            } catch (...) {
                spdlog::error("Failed to load tank.");
            }

            /*
             * Radar
             */
            static auto radarClientMaterial = threepp::MeshBasicMaterial::create();
            radarClientMaterial->color = threepp::Color(0xffffff);
            radarClientMaterial->transparent = false;
            radarClientMaterial->depthTest = false;
            auto radarGeometry = threepp::CircleGeometry::create(RADAR_CLIENT_SIZE, 32);
            auto radarClient = threepp::Mesh::create(radarGeometry, radarClientMaterial);
            radarScene->add(radarClient);
            radarClient->rotation.x = -(glm::pi<float>()) / 2;
            radarMesh = radarClient;
        }

        ~Client() override {
            this->scene->remove(*mesh);
            this->radarScene->remove(*radarMesh);
        }

        void update() override {
            if (state.alive) {
                mesh->visible = true;
                radarMesh->visible = true;

                Location loc = state.location;
                loc.position.y -= 1; // Adjust height so tank is above ground
                mesh->position = toInternal(loc.position);
                mesh->quaternion = toInternal(loc.rotation);

                radarMesh->position = toInternal(loc.position);
            } else {
                mesh->visible = false;
                radarMesh->visible = false;
            }
        }
    };

}