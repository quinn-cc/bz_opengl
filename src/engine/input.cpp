#include "engine/input.hpp"
#include "engine/types.hpp"

Input::Input(GLFWwindow *window) {
    this->window = window;
    this->inputState = {};
}

void Input::update() {
    inputState.movement = { 0, 0 };
    inputState.fire = false;
    inputState.jump = false;
    inputState.spawn = false;
    inputState.quickQuit = false;

    if (glfwGetKey(window, GLFW_KEY_LEFT) || glfwGetKey(window, GLFW_KEY_J))
        inputState.movement.x -= 1;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) || glfwGetKey(window, GLFW_KEY_L))
        inputState.movement.x += 1;

    // Movement
    if (glfwGetKey(window, GLFW_KEY_UP) || glfwGetKey(window, GLFW_KEY_I))
        inputState.movement.y += 1;
    if (glfwGetKey(window, GLFW_KEY_DOWN) || glfwGetKey(window, GLFW_KEY_K))
        inputState.movement.y -= 1;

    // Jump
    if (glfwGetKey(window, GLFW_KEY_SPACE)) {
        inputState.jump = true;
    }

    glfwPollEvents();
}

const InputState &Input::getInputState() const {
    return inputState;
}