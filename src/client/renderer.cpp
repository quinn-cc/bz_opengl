#include "renderer.hpp"
#include <glm/glm.hpp>
#include <threepp/threepp.hpp>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <memory>
#include "client.hpp"
#include "shot.hpp"
#include "player.hpp"

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

    spdlog::debug("About to init renderer");

    renderer = new threepp::GLRenderer({800.0f, 600.0f});

    scene = threepp::Scene::create();
    camera = threepp::PerspectiveCamera::create(75, 800.f/600.f, 0.1f, 1000.f);

    auto grid = threepp::GridHelper::create(10, 10); // size=10 units, divisions=10
    scene->add(grid);

    auto light = threepp::AmbientLight::create(0xffffff, 0.5f);
    scene->add(light);
}

bool Renderer::ShouldClose() {
    return glfwWindowShouldClose(window);
}

void Renderer::Close() {
    glfwTerminate();
}

void Renderer::BeginFrame() {
    glfwPollEvents();
}

void Renderer::EndFrame() {
    renderer->render(*scene, *camera);
    glfwSwapBuffers(window);
}

float Renderer::GetDeltaTime() {
    return deltaTime;
}

void Renderer::Update() {
    std::chrono::time_point<std::chrono::system_clock> currTime = std::chrono::system_clock::now();  
    deltaTime = std::chrono::duration<double>(currTime - lastFrameTime).count();
    lastFrameTime = currTime;

    glm::vec3 lookDir = Player::GetInstance().GetForwardVector();
    glm::vec3 offset = { 0.0f, 0.5f, 0.0f };
    glm::vec3 pos = Player::GetInstance().GetLocation().position + offset;
    camera->position = toInternal(pos);
    glm::vec3 total = pos + lookDir;
    threepp::Vector3 atVec = toInternal(total);
    camera->lookAt(atVec);
}

void Renderer::Draw(Client *client) {
    Location loc = client->GetInterpolatedLocation();

    if (client->hasMesh == false) {
        auto geometry = threepp::BoxGeometry::create(1, 1, 1);
        auto material = threepp::MeshBasicMaterial::create();
        material->color = threepp::Color(0x00ff00); // green
        client->mesh = threepp::Mesh::create(geometry, material);
        client->hasMesh = true;
    }

    scene->add(client->mesh);
    client->mesh->position = toInternal(loc.position);
}

void Renderer::Draw(Shot *shot) {
    // float dt = GetFrameTime();
    
    // glm::vec3 pos = shot->GetPosition();
    // DrawSphere(toInternal(pos), BULLET_SIZE, GREEN);
}


//Model model;

//void Render_Init() {
    //model = LoadModel("/home/quinn/bz_opengl/data/models/tank/tank.gltf");     // Supports OBJ, GLTF, IQM
    //Texture2D texture = LoadTexture("/home/quinn/bz_opengl/data/models/tank/quinn/bz_opengl/01_-_Default_baseColor.png");
    //model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    //texture = LoadTexture("/home/quinn/bz_opengl/data/models/tank/quinn/bz_opengl/02_-_Default_baseColor.png");
    //model.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    //texture = LoadTexture("/home/quinn/bz_opengl/data/models/tank/quinn/bz_opengl/03_-_Default_baseColor.png");
    //model.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
//}