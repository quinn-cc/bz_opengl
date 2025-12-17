#include "engine/render.hpp"

Render::Render(GLFWwindow *window) {
    // Initialize renderer, scene, camera, etc.
}

Render::~Render() {
    // Cleanup resources
}

render_id Render::create(std::string modelPath) {
    // Load model and add to scene
    static render_id nextId = 1;
    render_id id = nextId++;

    

    return id;
}
