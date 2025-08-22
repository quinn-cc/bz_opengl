#include "input.hpp"
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

/*
 * Private
 */

void Input::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        // Fire
        if (key == GLFW_KEY_F) {
            inputMap.fire = true;
        }

        // Spawn
        if (key == GLFW_KEY_U) {
            inputMap.spawn = true;
        }
    }
}

/*
 * Public
 */

void Input::Init(GLFWwindow *window) {
    this->window = window;
    inputMap.movement = { 0, 0 };
    
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto* self = static_cast<Input*>(glfwGetWindowUserPointer(window));
        self->keyCallback(window, key, scancode, action, mods);
    });
}

void Input::Update() {
    inputMap.movement = { 0, 0 };
    inputMap.fire = false;
    inputMap.jump = false;
    inputMap.spawn = false;

    if (glfwGetKey(window, GLFW_KEY_LEFT))
        inputMap.movement.x -= 1;
    if (glfwGetKey(window, GLFW_KEY_RIGHT))
        inputMap.movement.x += 1;

    // Movement
    if (glfwGetKey(window, GLFW_KEY_UP))
        inputMap.movement.y += 1;
    if (glfwGetKey(window, GLFW_KEY_DOWN))
        inputMap.movement.y -= 1;

    // Jump
    if (glfwGetKey(window, GLFW_KEY_SPACE)) {
        inputMap.jump = true;
    }

    glfwPollEvents();
}

const InputMap &Input::GetInputMap() const {
    return inputMap;
}

void Input::Close() {
    // No specific close actions needed for Input
}