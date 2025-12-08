#pragma once
#include "IRender.hpp"
#include <threepp/loaders/AssimpLoader.hpp>

namespace Render {

    class World : public IRender {
    public:
        World(
            GLFWwindow &window,
            std::shared_ptr<threepp::Scene> scene,
            std::shared_ptr<threepp::Scene> radarScene
        ) : IRender(window, scene, radarScene) {
            threepp::AssimpLoader assimpLoader;

            auto world = assimpLoader.load("../data/world2.glb");
            world->traverseType<threepp::Mesh>([&](threepp::Mesh& child) {
                child.castShadow = true;
                child.receiveShadow = true;
            });
            scene->add(world);

            auto light = threepp::AmbientLight::create(0xffffff, 0.5f);
            scene->add(light);

            auto dir = threepp::DirectionalLight::create(threepp::Color(0xffffff), 1.0f); // white, full intensity
            dir->position.set(150, 50, 150);
            dir->castShadow = true;
            dir->shadow->mapSize.set(2048, 2048);
            auto shadowCam = dynamic_cast<threepp::OrthographicCamera*>(dir->shadow->camera.get());
            shadowCam->left   = -50;
            shadowCam->right  =  50;
            shadowCam->top    =  50;
            shadowCam->bottom = -50;
            shadowCam->updateProjectionMatrix();
            scene->add(dir);
        }
    };

}