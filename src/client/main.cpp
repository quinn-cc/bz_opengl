#include <GLFW/glfw3.h>
#include <memory>
#include "spdlog/spdlog.h"
#include "engine/client_engine.hpp"
#include "game.hpp"
#include "client/client_cli_options.hpp"
#include "client/config_client.hpp"
#include "client/server_browser_controller.hpp"
#include "client/server_connector.hpp"

TimeUtils::time lastFrameTime;

#define MIN_DELTA_TIME (1.0f / 120.0f)

int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::trace);

    if (!glfwInit()) {
        spdlog::error("GLFW failed to initialize");
        exit(1);
    }

    const ClientCLIOptions cliOptions = ParseClientCLIOptions(argc, argv);

    constexpr const char *kClientConfigPath = "../data/config_client.json";
    ClientConfig clientConfig = ClientConfig::Load(kClientConfigPath);

    spdlog::trace("GLFW initialized successfully");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    

    GLFWwindow *window = glfwCreateWindow(800, 600, "BZFlag v3", nullptr, nullptr);
    //glfwSetWindowUserPointer(window, userPointer);
    if (!window) {
        spdlog::error("GLFW window failed to create");
        glfwTerminate();
        exit(1);
    }


    spdlog::trace("GLFW window created successfully");
    glfwMakeContextCurrent(window);
    spdlog::trace("GLFW context made current");

    glEnable(GL_MULTISAMPLE);
    glfwSwapInterval(1); // Enable vsync


    spdlog::info("GLFW_SAMPLES attrib = {}", glfwGetWindowAttrib(window, GLFW_SAMPLES));

    spdlog::info("GL_VENDOR   = {}", (const char*)glGetString(GL_VENDOR));
    spdlog::info("GL_RENDERER = {}", (const char*)glGetString(GL_RENDERER));
    spdlog::info("GL_VERSION  = {}", (const char*)glGetString(GL_VERSION));

    int sb = 0, s = 0;
    glGetIntegerv(GL_SAMPLE_BUFFERS, &sb);
    glGetIntegerv(GL_SAMPLES, &s);
    spdlog::info("GL_SAMPLE_BUFFERS={}, GL_SAMPLES={}", sb, s);

    ClientEngine engine(window);
    spdlog::trace("ClientEngine initialized successfully");

    std::unique_ptr<Game> game;
    ServerConnector serverConnector(engine, cliOptions.playerName, cliOptions.worldDir, game);
    ServerBrowserController serverBrowser(engine, clientConfig, cliOptions.connectAddr, cliOptions.connectPort, serverConnector);

    if (cliOptions.addrExplicit) {
        serverConnector.connect(cliOptions.connectAddr, cliOptions.connectPort);
    }

    lastFrameTime = TimeUtils::GetCurrentTime();

    spdlog::trace("Starting main loop");

    while (!glfwWindowShouldClose(window)) {
        TimeUtils::time currTime = TimeUtils::GetCurrentTime();  
        TimeUtils::duration deltaTime = TimeUtils::GetElapsedTime(lastFrameTime, currTime);

        if (deltaTime < MIN_DELTA_TIME) {
            TimeUtils::sleep(MIN_DELTA_TIME - deltaTime);
            continue;
        }

        lastFrameTime = currTime;

        engine.earlyUpdate(deltaTime);

        if (!game) {
            serverBrowser.update();
        }

        if (game) {
            game->earlyUpdate(deltaTime);
        }

        engine.step(deltaTime);

        if (game) {
            game->lateUpdate(deltaTime);
        }

        engine.lateUpdate(deltaTime);

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}