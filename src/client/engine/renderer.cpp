#include "renderer.hpp"
#include "geometry.hpp"
#include <glm/glm.hpp>
#include <threepp/threepp.hpp>
#include <threepp/loaders/AssimpLoader.hpp>
#include <threepp/lights/DirectionalLightShadow.hpp>
#include <threepp/cameras/OrthographicCamera.hpp>
#include <threepp/renderers/GLRenderTarget.hpp>
#include <threepp/materials/ShaderMaterial.hpp>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <memory>
#include "player.hpp"
#include "client.hpp"
#include "shot.hpp"
#include "timeutils.hpp"
#include "userpointer.hpp"
#include "game.hpp"


threepp::Vector3 Renderer::toInternal(const glm::vec3 &v) {
    return threepp::Vector3(v.x, v.y, v.z);
}

threepp::Quaternion Renderer::toInternal(const glm::quat &q) {
    return threepp::Quaternion(q.x, q.y, q.z, q.w);
}

glm::vec3 Renderer::toGLM(const threepp::Vector3 &v) {
    return glm::vec3{ v.x, v.y, v.z };
}

glm::quat Renderer::toGLM(const threepp::Quaternion &q) {
    return glm::quat{ q.x, q.y, q.z, q.w };
}

void Renderer::Init(Game *game, GLFWwindow* window) {
    this->window = window;
    this->game = game;
    closed = false;

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
    camera = threepp::PerspectiveCamera::create(CAMERA_FOV, 800.f/600.f, 0.1f, 1000.f);

    

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

    lastFrameTime = TimeUtils::GetCurrentTime();
    deltaTime = 0.0001f;

    /* 
     * Radar
     */
    // Radar camera parameters
    float radarWidth = RADAR_CAMERA_SIZE;
    float radarHeight = RADAR_CAMERA_SIZE;

    radarCamera = threepp::OrthographicCamera::create(
        -radarWidth/2, radarWidth/2,
        radarHeight/2, -radarHeight/2,
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

    auto radarWorld = assimpLoader.load("../data/world2.glb");
    radarMat = threepp::ShaderMaterial::create();
    radarMat->uniforms["playerY"] = threepp::Uniform(0.0f);
    radarMat->uniforms["jumpHeight"] = threepp::Uniform(JUMP_HEIGHT);
    radarMat->vertexShader = loadShader("../data/shaders/radar.vert");
    radarMat->fragmentShader = loadShader("../data/shaders/radar.frag");
    radarWorld->traverseType<threepp::Mesh>([&](threepp::Mesh& child) {
        child.setMaterial(radarMat);
    });
    radarScene = threepp::Scene::create();
    radarScene->add(radarWorld);

    /*
     * Radar player indicator (a simple circle on the radar)
     */
    auto geometry = threepp::CircleGeometry::create(0.7, 32); // radius 50 px
    auto material = threepp::MeshBasicMaterial::create();
    material->color = threepp::Color(0xffffff);
    material->transparent = false;
    material->depthTest = false; // always drawn on top
    radarPlayer = threepp::Mesh::create(geometry, material);
    radarScene->add(radarPlayer);
    radarPlayer->rotation.x = -(glm::pi<float>()) / 2; // face up

    /*
     * Field of view indicator on radar
     */
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

    /*
     * Radar shots
     */
    radarShotMaterial = threepp::MeshBasicMaterial::create();
    radarShotMaterial->color = threepp::Color(0xffffff);
    radarShotMaterial->transparent = false;
    radarShotMaterial->depthTest = false;

    /*
     * Client indicators
     */
    radarClientMaterial = threepp::MeshBasicMaterial::create();
    radarClientMaterial->color = threepp::Color(0xffffff);
    radarClientMaterial->transparent = false;
    radarClientMaterial->depthTest = false;
}

void Renderer::updateFovLines() {
    float px = game->player.GetLocation().position.x;
    float pz = game->player.GetLocation().position.z;

    glm::vec3 forward = game->player.GetForwardVector();
    forward = glm::normalize(glm::vec3(forward.x, 0, forward.z)); // flatten to XZ plane

    float halfFov = glm::radians(getVFOV() / 2.0f);
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

    // Add to radar
    auto radarGeometry = threepp::CircleGeometry::create(RADAR_CLIENT_SIZE, 32);
    auto radarClient = threepp::Mesh::create(radarGeometry, radarClientMaterial);
    radarScene->add(radarClient);
    radarClient->rotation.x = -(glm::pi<float>()) / 2;
    radarClientMeshes[client] = radarClient;
}

void Renderer::RemoveClient(Client *client) {
    // Remove from main scene
    auto &mesh = *(clientMeshes[client]);
    scene->remove(mesh);
    clientMeshes.erase(client);

    // Remove from radar scene
    threepp::Mesh &radarMesh = *(radarClientMeshes[client]);
    radarScene->remove(radarMesh);
    radarClientMeshes.erase(client);
}

void Renderer::AddShot(Shot *shot) {
    // Add to main scene
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

    // Add to radar
    auto radarGeometry = threepp::CircleGeometry::create(RADAR_SHOT_SIZE, 32);
    auto radarShot = threepp::Mesh::create(radarGeometry, radarShotMaterial);
    radarScene->add(radarShot);
    radarShot->rotation.x = -(glm::pi<float>()) / 2;
    radarShotMeshes[shot] = radarShot;
}

void Renderer::RemoveShot(Shot *shot) {
    // Remove from main scene
    threepp::Mesh &mesh = *(shotMeshes[shot]);
    scene->remove(mesh);
    shotMeshes.erase(shot);

    // Remove from radar scene
    threepp::Mesh &radarMesh = *(radarShotMeshes[shot]);
    radarScene->remove(radarMesh);
    radarShotMeshes.erase(shot);
}

unsigned int Renderer::GetRadarTextureId() {
    unsigned int framebuffer = renderer->getGlTextureId(*radarRenderTarget->texture).value();
    return framebuffer;
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

    glm::vec3 radarPos = game->player.GetLocation().position + glm::vec3(0, RADAR_HEIGHT, 0);
    radarCamera->position = toInternal(radarPos);
    glm::quat radarQuat = game->player.GetLocation().rotation;
    glm::quat pitchDown = glm::angleAxis(glm::radians(-90.0f), glm::vec3(1, 0, 0));
    glm::quat pitchAround = glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 0, 1));
    radarQuat = radarQuat * pitchDown * pitchAround; // apply pitch in local space
    radarCamera->quaternion = toInternal(radarQuat);


    // Set the client tank meshes location
    for (Client *client : game->clients) {
        if (client->IsAlive()) {
            clientMeshes[client]->visible = true;
            radarClientMeshes[client]->visible = true;

            Location loc = client->GetInterpolatedLocation();
            loc.position.y -= 1; // Adjust height so tank is above ground
            clientMeshes[client]->position = toInternal(loc.position);
            clientMeshes[client]->quaternion = toInternal(loc.rotation);

            radarClientMeshes[client]->position = toInternal(loc.position);
        } else {
            clientMeshes[client]->visible = false;
            radarClientMeshes[client]->visible = false;
        }
    }

    // Set the shot meshes location
    for (const auto& [shot, mesh] : shotMeshes) {
        glm::vec3 pos = shot->GetPosition();
        shotMeshes[shot]->position = toInternal(pos);
        radarShotMeshes[shot]->position = toInternal(pos);
    }

    radarMat->uniforms["playerY"].setValue(game->player.GetLocation().position.y);
    glm::vec3 playerPos = game->player.GetLocation().position;
    radarPlayer->position.set(playerPos.x, playerPos.y, playerPos.z);
    updateFovLines();

    int screenWidth = renderer->size().width();
    int screenHeight = renderer->size().height();

    renderer->setRenderTarget(nullptr);
    renderer->setViewport(0, 0, screenWidth, screenHeight);
    renderer->setClearColor(threepp::Color::skyblue);
    renderer->render(*scene, *camera);

    renderer->setRenderTarget(radarRenderTarget.get()); // Ensure we're rendering to the screen
    renderer->setViewport(0, 0, RADAR_SIZE, RADAR_SIZE);
    renderer->setClearColor(threepp::Color::black, 0.3f);
    renderer->clearDepth();
    renderer->render(*radarScene, *radarCamera);

     // Ensure we're rendering to the screen
    renderer->setRenderTarget(nullptr);
    renderer->setViewport(0, 0, screenWidth, screenHeight);
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

