#include "engine/engine.hpp"
#include "engine/types.hpp"
#include "spdlog/spdlog.h"
#include <functional>

/*
 * Client constructor
 */
Engine::Engine(Mode mode, GLFWwindow *window) {
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
    } else {
        spdlog::error("Server mode requires server port and callbacks. Use the other constructor.");
    }
}

/*
 * Server constructor
 */
Engine::Engine(Mode mode, GLFWwindow *window, uint16_t serverPort,
    std::function<void(client_id)> connectionCallback, 
    std::function<void(client_id)> disconnectionCallback) {

    this->mode = MODE_CLIENT;
    this->window = nullptr;

    if (mode == MODE_SERVER) {
        serverNetwork = new ServerNetwork(
            serverPort,
            connectionCallback,
            disconnectionCallback
        );
        physics = new Physics();
    } else {
        spdlog::error("Client mode requires a valid GLFWwindow pointer. Use the other constructor.");
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

void Engine::update(TimeUtils::duration deltaTime) {
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