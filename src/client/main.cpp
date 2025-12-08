#include "engine/renderer.hpp"
#include "engine/physics.hpp"
#include "engine/player.hpp"
#include "engine/client.hpp"
#include "engine/shot.hpp"
#include "engine/client_network.hpp"

std::vector<Client *> clients;
std::vector<Shot *> shots;

int main() {
    window = glfwCreateWindow(800, 600, "BZFlag v3", nullptr, nullptr);
    //glfwSetWindowUserPointer(window, userPointer);
    if (!window) {
        spdlog::error("GLFW window failed to create");
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(window);

    Engine engine(MODE_CLIENT, window);
    

    while (true) {
        engine.update();

        // Does minimal possible to link network messages to game 
        
    }
}