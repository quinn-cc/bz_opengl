#include "input.hpp"
#include "renderer.hpp"
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

/*
 * Priavte
 */

void Input::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Input &input = GetInstance();

    if (action == GLFW_PRESS) {
        // Fire
        if (key == GLFW_KEY_F) {
            input.fireReady = true;
        }

        
    }
}

/*
 * Public
 */

Input &Input::GetInstance() {
    static Input instance;
    return instance;
}

Input::Input() {
    movement = { 0, 0 };
}

void Input::Init() {
    glfwSetKeyCallback(Renderer::GetInstance().window, keyCallback);
}

void Input::Update() {
    movement = { 0, 0 };
    fireReady = false;
    jumpReady = false;
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

    // Spawn
    if (glfwGetKey(window, GLFW_KEY_U)) {
        requestingSpawn = true;
    }

    // Jump
    if (glfwGetKey(window, GLFW_KEY_SPACE)) {
        jumpReady = true;
    }

    glfwPollEvents();
}

bool Input::FireReady() {
    return fireReady;
}

bool Input::JumpReady() {
    return jumpReady;
}

bool Input::RequestingSpawn() {
    return requestingSpawn;
}

glm::vec2 Input::GetMovement() {
    return movement;
}