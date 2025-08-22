#pragma once
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "types.hpp"

class Input {
private:
    GLFWwindow* window;
    InputMap inputMap;
    void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

public:
    void Init(GLFWwindow *window);
    void Update();
    void Close();
    
    const InputMap &GetInputMap() const;
};