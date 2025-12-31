#pragma once
#include "gui.hpp"
#include "input.hpp"
#include "physics.hpp" 
#include "renderer.hpp"
#include "networker.hpp"
#include "game.hpp"

class Engine {
private:
    Game *game;

public:
    GUI gui;
    Input input;
    Physics physics;
    Renderer renderer;
    Networker networker;

    void Init(Game *game, GLFWwindow* window) {
        this->game = game;
        renderer.Init(game, window);
        gui.Init(game, window);
        input.Init(window);
        physics.Init(game);
        networker.Init();
    }

    void Close() {
        gui.Close();
        input.Close();
        physics.Close();
        renderer.Close();
        networker.Close();
    }
};