#pragma once
#include "engine/server_network.hpp"
#include "engine/client_network.hpp"
#include "engine/render.hpp"
#include "engine/physics.hpp"
#include "engine/input.hpp"
#include "engine/gui.hpp"

enum Mode {
    MODE_CLIENT,
    MODE_SERVER
};

class Engine {
private:
    GLFWwindow *window;
    TimeUtils::time lastFrameTime;
    Mode mode;

public:
    ClientNetwork *clientNetwork = nullptr;
    ServerNetwork *serverNetwork = nullptr;
    Render *render = nullptr;
    Physics *physics = nullptr;
    Input *input = nullptr;
    GUI *gui = nullptr;

    Engine(Mode mode, GLFWwindow *window);
    ~Engine();

    void update();
};