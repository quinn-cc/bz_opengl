#include <GLFW/glfw3.h>
#include "spdlog/spdlog.h"
#include "engine/engine.hpp"
#include "game.hpp"

TimeUtils::time lastFrameTime;

int main() {
    GLFWwindow *window = glfwCreateWindow(800, 600, "BZFlag v3", nullptr, nullptr);
    //glfwSetWindowUserPointer(window, userPointer);
    if (!window) {
        spdlog::error("GLFW window failed to create");
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(window);

    Engine engine(MODE_CLIENT, window);
    Game game(engine);

    lastFrameTime = TimeUtils::GetCurrentTime();

    while (true) {
        TimeUtils::time currTime = TimeUtils::GetCurrentTime();  
        TimeUtils::duration deltaTime = TimeUtils::GetElapsedTime(lastFrameTime, TimeUtils::GetCurrentTime());
        deltaTime = std::max(deltaTime, 0.0001f);
        lastFrameTime = currTime;

        engine.update(deltaTime);
        game.update(deltaTime);
    }
}