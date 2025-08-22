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

    void Init(Game *game) {
        this->game = game;
        renderer.Init(game);
        gui.Init(game, renderer.window);
        input.Init(renderer.window);
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