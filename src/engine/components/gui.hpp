#pragma once
#include <GLFW/glfw3.h>

class GUI {
    friend class ClientEngine;

private:
    GLFWwindow *window;
    void update();

    GUI(GLFWwindow *window);
    ~GUI();

public:
    void drawTexture(unsigned int textureId);
};