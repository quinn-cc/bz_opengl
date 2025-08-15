#pragma once
#include <glm/glm.hpp>

class Input {
private:
    bool fireReady = false;
    bool requestingSpawn = false;
    glm::vec2 movement;

public:
    static Input &GetInstance();

    Input();
    void Update();    
    bool FireReady();
    bool RequestingSpawn();
    glm::vec2 GetMovement();
};