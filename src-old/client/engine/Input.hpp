#pragma once
#include <GLFW/glfw3.h>
#include "userpointer.hpp"
#include "types.hpp"

namespace Input {

    class Input {
    private:
        GLFWwindow &window;
        InputMap inputMap;

    public:
        Input(GLFWwindow &window) : window(window) {
            auto* userPointer = static_cast<GLFWUserPointer*>(glfwGetWindowUserPointer(&window));
            userPointer->input = this;

            glfwSetKeyCallback(&window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
                auto* userPointer = static_cast<GLFWUserPointer*>(glfwGetWindowUserPointer(window));
                userPointer->input->keyCallback(window, key, scancode, action, mods);
            });
        }

        void poll() {
            inputMap.movement = { 0, 0 };
            inputMap.fire = false;
            inputMap.jump = false;
            inputMap.spawn = false;
            inputMap.quickQuit = false;

            if (glfwGetKey(&window, GLFW_KEY_LEFT) || glfwGetKey(&window, GLFW_KEY_J))
                inputMap.movement.x -= 1;
            if (glfwGetKey(&window, GLFW_KEY_RIGHT) || glfwGetKey(&window, GLFW_KEY_L))
                inputMap.movement.x += 1;

            // Movement
            if (glfwGetKey(&window, GLFW_KEY_UP) || glfwGetKey(&window, GLFW_KEY_I))
                inputMap.movement.y += 1;
            if (glfwGetKey(&window, GLFW_KEY_DOWN) || glfwGetKey(&window, GLFW_KEY_K))
                inputMap.movement.y -= 1;

            // Jump
            if (glfwGetKey(&window, GLFW_KEY_SPACE)) {
                inputMap.jump = true;
            }

            glfwPollEvents();
        }

        const InputMap &getInputMap() const {
            return inputMap;
        }
    };

}