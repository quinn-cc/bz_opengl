#include <GLFW/glfw3.h>
#include <memory>
#include "spdlog/spdlog.h"
#include "engine/client_engine.hpp"
#include "game.hpp"
#include "client/client_cli_options.hpp"
#include "client/config_client.hpp"
#include "client/server/server_browser_controller.hpp"
#include "client/server/server_connector.hpp"

TimeUtils::time lastFrameTime;

namespace {
struct FullscreenState {
    bool active = false;
    int windowedX = 0;
    int windowedY = 0;
    int windowedWidth = 1280;
    int windowedHeight = 720;
};

void ToggleFullscreen(GLFWwindow *window, FullscreenState &state) {
    if (!window) {
        return;
    }

    if (!state.active) {
        glfwGetWindowPos(window, &state.windowedX, &state.windowedY);
        glfwGetWindowSize(window, &state.windowedWidth, &state.windowedHeight);

        GLFWmonitor *monitor = glfwGetWindowMonitor(window);
        if (!monitor) {
            monitor = glfwGetPrimaryMonitor();
        }

        if (!monitor) {
            return;
        }

        int monitorX = 0;
        int monitorY = 0;
        glfwGetMonitorPos(monitor, &monitorX, &monitorY);

        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        if (!mode) {
            return;
        }

        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
        glfwSetWindowPos(window, monitorX, monitorY);
        glfwSetWindowSize(window, mode->width, mode->height);

        glfwSwapInterval(1); // ensure vsync remains active after mode switch
        state.active = true;
    } else {
        const int restoreWidth = state.windowedWidth > 0 ? state.windowedWidth : 1280;
        const int restoreHeight = state.windowedHeight > 0 ? state.windowedHeight : 720;
        const int restoreX = state.windowedX;
        const int restoreY = state.windowedY;
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
        glfwSetWindowPos(window, restoreX, restoreY);
        glfwSetWindowSize(window, restoreWidth, restoreHeight);
        glfwSwapInterval(1); // reapply vsync after returning to windowed mode
        state.active = false;
    }
}
}

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

    

    GLFWwindow *window = glfwCreateWindow(1280, 720, "BZFlag v3", nullptr, nullptr);
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
    FullscreenState fullscreenState;

    std::unique_ptr<Game> game;
    ServerConnector serverConnector(engine, cliOptions.playerName, cliOptions.worldDir, game);
    ServerBrowserController serverBrowser(
        engine,
        clientConfig,
        kClientConfigPath,
        cliOptions.connectAddr,
        cliOptions.connectPort,
        serverConnector);

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

        if (engine.input->getInputState().toggleFullscreen) {
            ToggleFullscreen(window, fullscreenState);
        }

        if (auto disconnectEvent = engine.network->consumeDisconnectEvent()) {
            if (game) {
                game.reset();
            }
            serverBrowser.handleDisconnected(disconnectEvent->reason);
        }

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