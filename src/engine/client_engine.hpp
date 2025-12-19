#pragma once
#include "engine/types.hpp"
#include "engine/components/client_network.hpp"
#include "engine/components/render.hpp"
#include "engine/components/physics.hpp"
#include "engine/components/input.hpp"
#include "engine/components/gui.hpp"
#include "engine/user_pointer.hpp"
#include <string>

class ClientEngine {
private:
    GLFWwindow *window;
    GLFWUserPointer *userPointer;

public:
    ClientNetwork *network;
    Render *render;
    Physics *physics;
    Input *input;
    GUI *gui;

    ClientEngine(GLFWwindow *window);
    ~ClientEngine();

    void earlyUpdate(TimeUtils::duration deltaTime);
    void lateUpdate(TimeUtils::duration deltaTime);
};