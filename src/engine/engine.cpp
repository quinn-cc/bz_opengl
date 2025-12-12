#include "engine/engine.hpp"
#include "spdlog/spdlog.h"

Engine::Engine(Mode mode, GLFWwindow *window,
        std::optional<uint16_t> serverPort) {
    this->mode = mode;
    this->window = window;

    if (mode == MODE_CLIENT) {
        clientNetwork = new ClientNetwork();
        render = new Render(window);
        physics = new Physics();
        input = new Input(window);
        gui = new GUI();

        if (window == nullptr) {
            spdlog::error("Client mode requires a valid GLFWwindow pointer.");
            return;
        }
    } else if (mode == MODE_SERVER) {
        if (!serverPort.has_value()) {
            spdlog::error("Server mode requires a server port.");
            return;
        }
        serverNetwork = new ServerNetwork(
            serverPort.value(),
        );
        physics = new Physics();
    }
}

Engine::~Engine() {
    delete clientNetwork;
    delete serverNetwork;
    delete render;
    delete physics;
    delete input;
    delete gui;
}

void Engine::update() {
    if (mode == MODE_CLIENT) {
        clientNetwork->update();
        input->update();
        physics->update(deltaTime);
        render->beginFrame();
        gui->startFrame();
        gui->drawTexture(render->getRadarTextureId());
        gui->endFrame();
        render->endFrame();
    } else if (mode == MODE_SERVER) {
        serverNetwork->update();
        physics->update(deltaTime);
    }
}