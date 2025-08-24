#include "renderer.hpp"
#include "geometry.hpp"
#include <glm/glm.hpp>
#include <threepp/threepp.hpp>
#include <threepp/loaders/AssimpLoader.hpp>
#include <threepp/lights/DirectionalLightShadow.hpp>
#include <threepp/cameras/OrthographicCamera.hpp>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <memory>
#include "player.hpp"
#include "client.hpp"
#include "shot.hpp"
#include "timeutils.hpp"
#include "userpointer.hpp"

threepp::Vector3 Renderer::toInternal(glm::vec3 &v) {
    return threepp::Vector3(v.x, v.y, v.z);
}

threepp::Quaternion Renderer::toInternal(glm::quat &q) {
    return threepp::Quaternion(q.x, q.y, q.z, q.w);
}

glm::vec3 Renderer::toGLM(threepp::Vector3 &v) {
    return glm::vec3{ v.x, v.y, v.z };
}

glm::quat Renderer::toGLM(threepp::Quaternion &q) {
    return glm::quat{ q.x, q.y, q.z, q.w };
}

void Renderer::Init(Game *game, GLFWwindow* window) {
    this->window = window;
    this->game = game;
    closed = false;

    
    //glfwSwapInterval(1);
    auto* userPointer = static_cast<GLFWUserPointer*>(glfwGetWindowUserPointer(window));
    userPointer->renderer = this;

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int width, int height) {
        auto* userPointer = static_cast<GLFWUserPointer*>(glfwGetWindowUserPointer(w));
        userPointer->renderer->OnResize(width, height);
    });

    renderer = new threepp::GLRenderer({800.0f, 600.0f});
    renderer->setClearColor(threepp::Color(0x3399ff));
    renderer->shadowMap().enabled = true;
    renderer->shadowMap().type = threepp::ShadowMap::PFCSoft;
    renderer->toneMapping = threepp::ToneMapping::ACESFilmic;

    scene = threepp::Scene::create();
    camera = threepp::PerspectiveCamera::create(60, 800.f/600.f, 0.1f, 1000.f);


    /*
     * Scene
     */
    threepp::AssimpLoader assimpLoader;
    auto world = assimpLoader.load("../data/world2.glb");
    world->traverseType<threepp::Mesh>([&](threepp::Mesh& child) {
        child.castShadow = true;
        child.receiveShadow = true;
    });
    scene->add(world);

    // auto tmp = assimpLoader.load("../data/world.glb");
    // scene->add(tmp);

    auto light = threepp::AmbientLight::create(0xffffff, 0.5f);
    scene->add(light);

    auto dir = threepp::DirectionalLight::create(threepp::Color(0xffffff), 1.0f); // white, full intensity
    dir->position.set(150, 50, 150);
    dir->castShadow = true;
    dir->shadow->mapSize.set(2048, 2048);
    auto shadowCam = static_cast<threepp::OrthographicCamera*>(dir->shadow->camera.get());
    if (shadowCam) {
        shadowCam->left = -50;
        shadowCam->right = 50;
        shadowCam->top = 50;
        shadowCam->bottom = -50;
    }
    scene->add(dir);

    lastFrameTime = TimeUtils::GetCurrentTime();
    deltaTime = 0.0001f;
}

bool Renderer::ShouldClose() {
    return closed || glfwWindowShouldClose(window);
}

void Renderer::Close() {
    // Release threepp resources first
    clientMeshes.clear();
    shotMeshes.clear();
    scene.reset();
    camera.reset();
    if (renderer) {
        delete renderer;
        renderer = nullptr;
    }

    spdlog::debug("Closing GLFW window");
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
    closed = true;
}

TimeUtils::duration Renderer::GetDeltaTime() {
    TimeUtils::duration ret = deltaTime;
    ret = std::max(ret, 0.001f);
    return deltaTime;
}

void Renderer::AddClient(Client *client) {
    threepp::AssimpLoader loader;

    try {
        auto model = loader.load("../data/models/tank/tank.glb");
        clientMeshes[client] = model;
        model->scale.set(0.5f, 0.5f, 0.5f);
        model->traverseType<threepp::Mesh>([&](threepp::Mesh& child) {
            child.castShadow = true;
            child.receiveShadow = true;
        });
        scene->add(model);
    } catch (...) {
        spdlog::error("Failed to load tank.");
    }
   
}

void Renderer::RemoveClient(Client *client) {
    auto &mesh = *(clientMeshes[client]);
    scene->remove(mesh);
    clientMeshes.erase(client);
}

void Renderer::AddShot(Shot *shot) {
    auto geometry = threepp::SphereGeometry::create(BULLET_SIZE);
    auto material = threepp::MeshStandardMaterial::create();
    material->color = threepp::Color(0x00ffff); // green
    material->metalness = 0.5f;   // optional: 0 = non-metal, 1 = metal
    material->roughness = 0.5f;   // 0 = smooth, 1 = rough
    std::shared_ptr<threepp::Mesh> mesh = threepp::Mesh::create(geometry, material);
    mesh->castShadow = true;
    mesh->receiveShadow = true;
    shotMeshes[shot] = mesh;
    scene->add(mesh);
}

void Renderer::RemoveShot(Shot *shot) {
    threepp::Mesh &mesh = *(shotMeshes[shot]);
    scene->remove(mesh);
    shotMeshes.erase(shot);
}

void Renderer::Update() {
    std::chrono::time_point<std::chrono::system_clock> currTime = std::chrono::system_clock::now();  
    deltaTime = TimeUtils::GetElapsedTime(lastFrameTime, TimeUtils::GetCurrentTime());
    deltaTime = std::max(deltaTime, 0.0001f);
    lastFrameTime = currTime;

    glm::vec3 lookDir = game->player.GetForwardVector();
    glm::vec3 offset = { 0.0f, 0, 0.0f };
    glm::vec3 pos = game->player.GetLocation().position + offset;
    camera->position = toInternal(pos);
    glm::vec3 total = pos + lookDir;
    threepp::Vector3 atVec = toInternal(total);
    camera->lookAt(atVec);

    // Draw the meshes
    for (const auto& [client, mesh] : clientMeshes) {
        if (client->IsAlive()) {
            mesh->visible = true;
            Location loc = client->GetInterpolatedLocation();
            loc.position.y -= 1; // Adjust height so tank is above ground
            clientMeshes[client]->position = toInternal(loc.position);
            clientMeshes[client]->quaternion = toInternal(loc.rotation);
        } else {
            mesh->visible = false;
        }
    }

    for (const auto& [shot, mesh] : shotMeshes) {
        glm::vec3 pos = shot->GetPosition();
        shotMeshes[shot]->position = toInternal(pos);
    }

    renderer->render(*scene, *camera);
}

void Renderer::EndFrame() {
    glfwSwapBuffers(window);
}

void Renderer::OnResize(int width, int height) {
    if (renderer) renderer->setSize({width, height});
    if (camera) {
        camera->aspect = static_cast<float>(width) / static_cast<float>(height);
        camera->updateProjectionMatrix();
    }
}

