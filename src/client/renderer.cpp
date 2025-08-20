#include "renderer.hpp"
#include "networker.hpp"
#include <glm/glm.hpp>
#include <threepp/threepp.hpp>
#include <threepp/loaders/AssimpLoader.hpp>
#include <threepp/lights/DirectionalLightShadow.hpp>
#include <threepp/cameras/OrthographicCamera.hpp>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <memory>
#include "client.hpp"
#include "shot.hpp"
#include "player.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Renderer::GetInstance().OnResize(width, height);
}

Renderer &Renderer::GetInstance() {
    static Renderer instance;
    return instance;
}

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

void Renderer::Init() {
    closed = false;

    if (!glfwInit()) {
        spdlog::error("GLFW failed to initialize");
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 600, "BZFlag v3", nullptr, nullptr);
    if (!window) {
        spdlog::error("GLFW window failed to create");
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    renderer = new threepp::GLRenderer({800.0f, 600.0f});
    renderer->setClearColor(threepp::Color(0x3399ff));
    renderer->shadowMap().enabled = true;
    renderer->shadowMap().type = threepp::ShadowMap::PFCSoft;
    renderer->toneMapping = threepp::ToneMapping::ACESFilmic;

    scene = threepp::Scene::create();
    camera = threepp::PerspectiveCamera::create(75, 800.f/600.f, 0.1f, 1000.f);

    const auto planeGeometry = threepp::PlaneGeometry::create(100, 100);
    const auto planeMaterial = threepp::MeshLambertMaterial::create();
    threepp::TextureLoader loader;
    try {
        auto texture = loader.load("../data/textures/grass.jpg");
        texture->repeat.set(20, 20);  // Repeat 20 times in both directions
        texture->wrapS = threepp::TextureWrapping::Repeat;  // Enable horizontal wrapping
        texture->wrapT = threepp::TextureWrapping::Repeat;  // Enable vertical wrapping
        planeMaterial->map = texture;
    } catch (...) {
        spdlog::error("Failed to load grass texture");
    }
    planeMaterial->color = threepp::Color::lightgray;
    planeMaterial->side = threepp::Side::Double;
    auto plane = threepp::Mesh::create(planeGeometry, planeMaterial);
    plane->rotateX(threepp::math::degToRad(90));
    plane->receiveShadow = true;
    scene->add(plane);

    auto geometry = threepp::BoxGeometry::create(5, 5, 5);
    geometry->translate(10, 0, 10); // Move the box up so it sits on the ground
    const auto cubeMaterial = threepp::MeshStandardMaterial::create();
    auto cubeMesh = threepp::Mesh::create(geometry, cubeMaterial);
    cubeMesh->castShadow = true;
    cubeMesh->receiveShadow = true;
    scene->add(cubeMesh);

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

    Client::AddCallback_AddClient([this](Client *client){ this->OnClientAdd(client); });
    Client::AddCallback_RemoveClient([this](Client *client){ this->OnClientRemove(client); });
    Shot::AddCallback_AddShot([this](Shot *shot){ this->OnShotAdd(shot); });
    Shot::AddCallback_RemoveShot([this](Shot *shot){ this->OnShotRemove(shot); });
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

float Renderer::GetDeltaTime() {
    return deltaTime;
}

void Renderer::OnClientAdd(Client *client) {
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

void Renderer::OnClientRemove(Client *client) {
    auto &mesh = *(clientMeshes[client]);
    scene->remove(mesh);
    clientMeshes.erase(client);
}

void Renderer::OnShotAdd(Shot *shot) {
    auto geometry = threepp::SphereGeometry::create(0.5);
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

void Renderer::OnShotRemove(Shot *shot) {
    threepp::Mesh &mesh = *(shotMeshes[shot]);
    scene->remove(mesh);
    shotMeshes.erase(shot);
}

void Renderer::Update() {
    std::chrono::time_point<std::chrono::system_clock> currTime = std::chrono::system_clock::now();  
    deltaTime = std::chrono::duration<double>(currTime - lastFrameTime).count();
    lastFrameTime = currTime;

    glm::vec3 lookDir = Player::GetInstance().GetForwardVector();
    glm::vec3 offset = { 0.0f, 0, 0.0f };
    glm::vec3 pos = Player::GetInstance().GetLocation().position + offset;
    camera->position = toInternal(pos);
    glm::vec3 total = pos + lookDir;
    threepp::Vector3 atVec = toInternal(total);
    camera->lookAt(atVec);

    // Draw the meshes
    for (const auto& [client, mesh] : clientMeshes) {
        Location loc = client->GetInterpolatedLocation();
        loc.position.y -= 1; // Adjust height so tank is above ground
        clientMeshes[client]->position = toInternal(loc.position);
        clientMeshes[client]->quaternion = toInternal(loc.rotation);
    }

    for (const auto& [shot, mesh] : shotMeshes) {
        glm::vec3 pos = shot->GetPosition();
        shotMeshes[shot]->position = toInternal(pos);
    }

    renderer->render(*scene, *camera);
    glfwSwapBuffers(window);
}

void Renderer::OnResize(int width, int height) {
    if (renderer) renderer->setSize({width, height});
    if (camera) {
        camera->aspect = static_cast<float>(width) / static_cast<float>(height);
        camera->updateProjectionMatrix();
    }
}

