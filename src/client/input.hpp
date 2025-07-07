#pragma once
#include <glm/glm.hpp>

class Input {
private:
    bool fireReady = false;
    glm::vec2 movement;

public:
    static Input &GetInstance();

    Input();
    void Update();    
    bool FireReady();
    glm::vec2 GetMovement();
};