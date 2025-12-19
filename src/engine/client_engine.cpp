#include "engine/client_engine.hpp"
#include "engine/types.hpp"

ClientEngine::ClientEngine(GLFWwindow *window) {
    this->window = window;

    network = new ClientNetwork();
    render = new Render(window);
    physics = new Physics();
    input = new Input(window);
    gui = new GUI(window);

    userPointer = new GLFWUserPointer();
    glfwSetWindowUserPointer(window, userPointer);
}

ClientEngine::~ClientEngine() {
    delete network;
    delete render;
    delete physics;
    delete input;
    delete gui;
}

void ClientEngine::earlyUpdate(TimeUtils::duration deltaTime) {
    input->update();
    network->update();
}

void ClientEngine::lateUpdate(TimeUtils::duration deltaTime) {
    physics->update(deltaTime);
    render->update();

    glfwSwapBuffers(window);
}