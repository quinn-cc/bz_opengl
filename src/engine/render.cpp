#include "engine/render.hpp"
#include "threepp/loaders/AssimpLoader.hpp"
#include "spdlog/spdlog.h"

Render::Render(GLFWwindow *window) : renderer({800, 600}) {
    renderer.setClearColor(threepp::Color(0x3399ff));
    renderer.shadowMap().enabled = true;
    renderer.shadowMap().type = threepp::ShadowMap::PFCSoft;
    renderer.toneMapping = threepp::ToneMapping::ACESFilmic;
}

Render::~Render() {
    for (auto& [id, object] : objects) {
        destroy(id);
    }
}

render_id Render::create(std::string modelPath) {
    // Load model and add to scene
    static render_id nextId = 1;
    render_id id = nextId++;

    threepp::AssimpLoader loader;

    try {
        auto model = loader.load(modelPath);
        model->scale.set(0.5f, 0.5f, 0.5f);
        model->traverseType<threepp::Mesh>([&](threepp::Mesh& child) {
            child.castShadow = true;
            child.receiveShadow = true;
        });
        scene->add(model);
        objects[id] = model;
    } catch (...) {
        spdlog::error("Render::create: Failed to load model at path {}", modelPath);
    }
    
    return id;
}

void Render::destroy(render_id id) {
    auto it = objects.find(id);
    if (it != objects.end()) {
        scene->remove(*(it->second));
        objects.erase(it);
    } else {
        spdlog::error("Render::destroy: Invalid render_id {}", id);
    }
}

void Render::setPosition(render_id id, const glm::vec3 &position) {
    auto it = objects.find(id);
    if (it != objects.end()) {
        it->second->position.set(position.x, position.y, position.z);
    } else {
        spdlog::error("Render::setPosition: Invalid render_id {}", id);
    }
}

void Render::setRotation(render_id id, const glm::quat &rotation) {
    auto it = objects.find(id);
    if (it != objects.end()) {
        it->second->quaternion.set(rotation.x, rotation.y, rotation.z, rotation.w);
    } else {
        spdlog::error("Render::setRotation: Invalid render_id {}", id);
    }
}

void Render::setVisible(render_id id, bool visible) {
    auto it = objects.find(id);
    if (it != objects.end()) {
        it->second->visible = visible;
    } else {
        spdlog::error("Render::setVisible: Invalid render_id {}", id);
    }
}