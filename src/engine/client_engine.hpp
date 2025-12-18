#pragma once
#include "engine/types.hpp"
#include "engine/components/client_network.hpp"
#include "engine/components/render.hpp"
#include "engine/components/physics.hpp"
#include "engine/components/input.hpp"
#include "engine/components/gui.hpp"
#include <string>

class ClientEngine {
private:
    GLFWwindow *window;

public:
    ClientNetwork *network;
    Render *render;
    Physics *physics;
    Input *input;
    GUI *gui;

    ClientEngine(GLFWwindow *window);
    ~ClientEngine();

    void update(TimeUtils::duration deltaTime);
};