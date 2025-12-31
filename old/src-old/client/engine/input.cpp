#include "input.hpp"
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include "userpointer.hpp"

/*
 * Private
 */

void Input::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        // Fire
        if (key == GLFW_KEY_F || key == GLFW_KEY_E) {
            inputMap.fire = true;
        }

        // Spawn
        if (key == GLFW_KEY_U) {
            inputMap.spawn = true;
        }

        if (key == GLFW_KEY_F12) {
            inputMap.quickQuit = true;
        }
    }
}

/*
 * Public
 */

void Input::Init(GLFWwindow *window) {
    this->window = window;
    inputMap.movement = { 0, 0 };
    
    auto* userPointer = static_cast<GLFWUserPointer*>(glfwGetWindowUserPointer(window));
    userPointer->input = this;

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto* userPointer = static_cast<GLFWUserPointer*>(glfwGetWindowUserPointer(window));
        userPointer->input->keyCallback(window, key, scancode, action, mods);
    });
}

void Input::Update() {
    inputMap.movement = { 0, 0 };
    inputMap.fire = false;
    inputMap.jump = false;
    inputMap.spawn = false;
    inputMap.quickQuit = false;

    if (glfwGetKey(window, GLFW_KEY_LEFT) || glfwGetKey(window, GLFW_KEY_J))
        inputMap.movement.x -= 1;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) || glfwGetKey(window, GLFW_KEY_L))
        inputMap.movement.x += 1;

    // Movement
    if (glfwGetKey(window, GLFW_KEY_UP) || glfwGetKey(window, GLFW_KEY_I))
        inputMap.movement.y += 1;
    if (glfwGetKey(window, GLFW_KEY_DOWN) || glfwGetKey(window, GLFW_KEY_K))
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