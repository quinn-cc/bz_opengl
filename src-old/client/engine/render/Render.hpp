#pragma once
#include <memory>
#include "threepp/threepp.hpp"
#include "IRender.hpp"
#include "userpointer.hpp"
#include <threepp/cameras/OrthographicCamera.hpp>
#include <threepp/renderers/GLRenderTarget.hpp>
#include "engine/render/Shot.hpp"
#include "engine/render/Client.hpp"
#include "types.hpp"
#include <vector>
#include <map>

#define CAMERA_FOV 60.0f
#define SCREEN_WIDTH 800.0f
#define SCREEN_HEIGHT 600.0f
#define RADAR_SIZE 200
#define RADAR_CAMERA_SIZE 20.0f

namespace Render {

    class Render {
    private:
        const State::Player &playerState;
        GLFWwindow &window;
        threepp::GLRenderer renderer;
        std::vector<std::shared_ptr<IRender>> renderables;
        std::map<shot_id, std::shared_ptr<Shot>> shots;
        std::map<client_id, std::shared_ptr<Client>> clients;

        /*
         * Scene
         */
        std::shared_ptr<threepp::Scene> scene;
        std::shared_ptr<threepp::PerspectiveCamera> camera;

        /* 
         * Radar
         */
        std::shared_ptr<threepp::Scene> radarScene;
        std::shared_ptr<threepp::OrthographicCamera> radarCamera;
        std::shared_ptr<threepp::GLRenderTarget> radarRenderTarget;
        std::shared_ptr<threepp::FloatBufferAttribute> fovPositionAttr;

        void onWindowResize(int width, int height) {
            renderer.setSize({width, height});
            if (camera) {
                camera->aspect = static_cast<float>(width) / static_cast<float>(height);
                camera->updateProjectionMatrix();
            }
        }

        float getVFOV(std::shared_ptr<threepp::PerspectiveCamera> camera, float hFOV) {
            float vFOV = glm::radians(hFOV); // convert degrees to radians
            float aspect = camera->aspect;          // your camera aspect ratio
            float hFOV = 2.f * atan(tan(vFOV / 2.f) * aspect);
            hFOV = glm::degrees(hFOV);            // convert back to degrees if needed
            return hFOV;
        }

        void updateFovLines() {
            float px = playerState.location.position.x;
            float pz = playerState.location.position.z;

            glm::vec3 forward = playerState.getForwardVector();
            forward = glm::normalize(glm::vec3(forward.x, 0, forward.z)); // flatten to XZ plane

            float halfFov = glm::radians(getVFOV(camera, CAMERA_FOV) / 2.0f);
            float length = 300.0f;

            auto rotateAroundY = [](const glm::vec3& v, float angle) {
                float cs = cos(angle);
                float sn = sin(angle);
                return glm::vec3(
                    v.x * cs - v.z * sn,
                    v.y,
                    v.x * sn + v.z * cs
                );
            };

            glm::vec3 leftDir = rotateAroundY(forward, -halfFov);
            glm::vec3 rightDir = rotateAroundY(forward, halfFov);

            fovPositionAttr->setXYZ(0, px, 0, pz);
            fovPositionAttr->setXYZ(1, px + leftDir.x * length, 0, pz + leftDir.z * length);

            fovPositionAttr->setXYZ(2, px, 0, pz);
            fovPositionAttr->setXYZ(3, px + rightDir.x * length, 0, pz + rightDir.z * length);

            fovPositionAttr->needsUpdate();
        }

    public:
        Render(
            GLFWwindow &window,
            State::Player &playerState
        ) : 
            renderer({SCREEN_WIDTH, SCREEN_HEIGHT}),
            window(window), 
            playerState(playerState)
        {
            auto* userPointer = static_cast<GLFWUserPointer *>(glfwGetWindowUserPointer(&window));
            userPointer->render = this;

            glfwSetFramebufferSizeCallback(&window, [](GLFWwindow* w, int width, int height) {
                auto* userPointer = static_cast<GLFWUserPointer *>(glfwGetWindowUserPointer(w));
                userPointer->render->onWindowResize(width, height);
            });

            renderer.setClearColor(threepp::Color(0x3399ff));
            renderer.shadowMap().enabled = true;
            renderer.shadowMap().type = threepp::ShadowMap::PFCSoft;
            renderer.toneMapping = threepp::ToneMapping::ACESFilmic;

            // Scene
            scene = threepp::Scene::create();
            camera = threepp::PerspectiveCamera::create(CAMERA_FOV, SCREEN_WIDTH/SCREEN_HEIGHT, 0.1f, 1000.f);

            // Radar
            radarScene = threepp::Scene::create();
            radarCamera = threepp::OrthographicCamera::create(
                -RADAR_CAMERA_SIZE/2, RADAR_CAMERA_SIZE/2,
                RADAR_CAMERA_SIZE/2, -RADAR_CAMERA_SIZE/2,
                0.1f, 1000.0f
            );
            
            threepp::GLRenderTarget::Options options{};
            options.minFilter = threepp::Filter::Linear;
            options.magFilter = threepp::Filter::Linear;
            options.format = threepp::Format::RGBA;
            options.type = threepp::Type::UnsignedByte; // important
            options.depthBuffer = true;  // attach depth buffer
            options.stencilBuffer = false; // usually not needed
            radarRenderTarget = threepp::GLRenderTarget::create(RADAR_SIZE, RADAR_SIZE, options);

            // Field of view lines
            auto fovGeometry = threepp::BufferGeometry::create();
            std::vector<float> points(12, 0.0f);
            fovPositionAttr = threepp::FloatBufferAttribute::create(points, 3);
            fovGeometry->setAttribute("position", fovPositionAttr);
            auto fovMaterial = threepp::LineBasicMaterial::create();
            fovMaterial->color = threepp::Color(0xdddd00);  // yellow
            fovMaterial->depthTest = false;                 // always drawn on top
            fovMaterial->transparent = false;
            auto fovLines = threepp::LineSegments::create(fovGeometry, fovMaterial);
            radarScene->add(fovLines);
        }

        void update() {
            for (auto &iRender : renderables) {
                iRender->update();
            }

            int screenWidth = renderer.size().width();
            int screenHeight = renderer.size().height();

            renderer.setRenderTarget(nullptr);
            renderer.setViewport(0, 0, screenWidth, screenHeight);
            renderer.setClearColor(threepp::Color::skyblue);
            renderer.render(*scene, *camera);

            renderer.setRenderTarget(radarRenderTarget.get()); // Ensure we're rendering to the screen
            renderer.setViewport(0, 0, RADAR_SIZE, RADAR_SIZE);
            renderer.setClearColor(threepp::Color::black, 0.3f);
            renderer.clearDepth();
            renderer.render(*radarScene, *radarCamera);

            // Ensure we're rendering to the screen
            renderer.setRenderTarget(nullptr);
            renderer.setViewport(0, 0, screenWidth, screenHeight);
        }

        bool shouldClose() {
            return glfwWindowShouldClose(&window);
        }

        void addShot(const State::Shot &shot) {
            auto shotPtr = std::make_shared<Shot>(window, scene, radarScene, shot);
            shots.emplace(
                shot.id,
                shotPtr
            );
            renderables.push_back(shotPtr);
        }

        void removeShot(const State::Shot &shot) {
            auto shotPtr = shots.at(shot.id);
            renderables.erase(std::remove(renderables.begin(), renderables.end(), shotPtr), renderables.end());
            shots.erase(shot.id);
        }

        void addClient(const State::Client &client) {
            auto clientPtr = std::make_shared<Client>(window, scene, radarScene, client);
            clients.emplace(
                client.id,
                clientPtr
            );
            renderables.push_back(clientPtr);
        }

        void removeClient(const State::Client &client) {
            auto clientPtr = clients.at(client.id);
            renderables.erase(std::remove(renderables.begin(), renderables.end(), clientPtr), renderables.end());
            clients.erase(client.id);
        }

        unsigned int getRadarTextureId() const {
            unsigned int framebuffer = renderer.getGlTextureId(*radarRenderTarget->texture).value();
            return framebuffer;
        }
    };

}