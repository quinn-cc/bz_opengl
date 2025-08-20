#pragma once
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class Input {
private:
    bool fireReady = false;
    bool requestingSpawn = false;
    bool jumpReady = false;
    glm::vec2 movement;
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

public:
    static Input &GetInstance();

    Input();
    void Init();
    void Update();    
    bool FireReady();
    bool JumpReady();
    bool RequestingSpawn();
    glm::vec2 GetMovement();
};