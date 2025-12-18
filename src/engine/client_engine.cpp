#include "engine/client_engine.hpp"
#include "engine/types.hpp"

ClientEngine::ClientEngine(GLFWwindow *window) {
    this->window = window;

    network = new ClientNetwork();
    render = new Render(window);
    physics = new Physics();
    input = new Input(window);
    gui = new GUI(window);
}

ClientEngine::~ClientEngine() {
    delete network;
    delete render;
    delete physics;
    delete input;
    delete gui;
}

void ClientEngine::update(TimeUtils::duration deltaTime) {
    network->update();
    input->update();
    physics->update(deltaTime);
}