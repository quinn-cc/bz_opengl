#pragma once
#include "imgui.h"
#include <string>
#include "game.hpp"
#include <GLFW/glfw3.h>

class GUI {
private:
    Game *game;

public:
    void Init(Game *game, GLFWwindow *window);
    void Update();
    void Close();
};