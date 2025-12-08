#pragma once
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

typedef struct InputState {
    bool fire;
    bool spawn;
    bool jump;
    bool quickQuit;
    glm::vec2 movement;
} InputState;

class Input {
private:
    void update();

    friend class Engine;

public:
    Input(GLFWwindow *window);
    ~Input();
    
    const InputState &getInputState() const;
};