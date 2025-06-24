#pragma once
#include "raylib.h"
#include <string>

class Player {
public:
    std::string name;
    int clientId;
    Vector3 position;
    void Draw();
};