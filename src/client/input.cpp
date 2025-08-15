#include "input.hpp"
#include "renderer.hpp"

Input &Input::GetInstance() {
    static Input instance;
    return instance;
}

Input::Input() {
    movement = { 0, 0 };
}

void Input::Update() {
    movement = { 0, 0 };
    fireReady = false;
    requestingSpawn = false;
    GLFWwindow *window = Renderer::GetInstance().window;

    if (glfwGetKey(window, GLFW_KEY_LEFT))
        movement.x -= 1;
    if (glfwGetKey(window, GLFW_KEY_RIGHT))
        movement.x += 1;

    // Movement
    if (glfwGetKey(window, GLFW_KEY_UP))
        movement.y += 1;
    if (glfwGetKey(window, GLFW_KEY_DOWN))
        movement.y -= 1;

    // Fire
    if (glfwGetKey(window, GLFW_KEY_F)) {
        fireReady = true;
    }

    // Spawn
    if (glfwGetKey(window, GLFW_KEY_U)) {
        requestingSpawn = true;
    }
}

bool Input::FireReady() {
    return fireReady;
}

bool Input::RequestingSpawn() {
    return requestingSpawn;
}

glm::vec2 Input::GetMovement() {
    return movement;
}