#pragma once
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include "engine/types.hpp"

class Input {
    friend class Engine;

private:
    InputState inputState;
    GLFWwindow *window;

    Input(GLFWwindow *window);
    ~Input() = default;

    void update();

public:
    const InputState &getInputState() const;
};