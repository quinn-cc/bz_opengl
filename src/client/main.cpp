#include <GLFW/glfw3.h>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <initializer_list>
#include <memory>
#include <string>
#include "spdlog/spdlog.h"
#include "engine/client_engine.hpp"
#include "game.hpp"
#include "client/client_cli_options.hpp"
#include "client/config_client.hpp"
#include "client/server/server_browser_controller.hpp"
#include "client/server/server_connector.hpp"
#include "common/data_path_resolver.hpp"

TimeUtils::time lastFrameTime;

namespace {
struct FullscreenState {
    bool active = false;
    int windowedX = 0;
    int windowedY = 0;
    int windowedWidth = 1280;
    int windowedHeight = 720;
};

void ToggleFullscreen(GLFWwindow *window, FullscreenState &state, bool vsyncEnabled) {
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

        glfwSwapInterval(vsyncEnabled ? 1 : 0); // reapply configured vsync after mode switch
        state.active = true;
    } else {
        const int restoreWidth = state.windowedWidth > 0 ? state.windowedWidth : 1280;
        const int restoreHeight = state.windowedHeight > 0 ? state.windowedHeight : 720;
        const int restoreX = state.windowedX;
        const int restoreY = state.windowedY;
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
        glfwSetWindowPos(window, restoreX, restoreY);
        glfwSetWindowSize(window, restoreWidth, restoreHeight);
        glfwSwapInterval(vsyncEnabled ? 1 : 0); // reapply configured vsync after returning
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

    const std::filesystem::path clientUserConfigPathFs = bz::data::EnsureUserConfigFile("config.json");
    const std::vector<bz::data::ConfigLayerSpec> clientConfigSpecs = {
        {"common/config.json", "data/common/config.json", spdlog::level::err, true},
        {"client/config.json", "data/client/config.json", spdlog::level::err, true},
        {clientUserConfigPathFs, "user config", spdlog::level::debug, false}
    };
    bz::data::InitializeConfigCache(clientConfigSpecs);

    auto readBoolConfig = [](std::initializer_list<const char*> paths, bool defaultValue) {
        for (const char* path : paths) {
            if (const auto* value = bz::data::ConfigValue(path)) {
                if (value->is_boolean()) {
                    return value->get<bool>();
                }
                if (value->is_number_integer()) {
                    return value->get<long long>() != 0;
                }
                if (value->is_number_float()) {
                    return value->get<double>() != 0.0;
                }
                if (value->is_string()) {
                    std::string text = value->get<std::string>();
                    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
                    if (text == "true" || text == "1" || text == "yes" || text == "on") {
                        return true;
                    }
                    if (text == "false" || text == "0" || text == "no" || text == "off") {
                        return false;
                    }
                }
                spdlog::warn("Client startup config '{}' cannot be interpreted as boolean", path);
            }
        }
        return defaultValue;
    };

    auto readUInt16Config = [](std::initializer_list<const char*> paths, uint16_t defaultValue) {
        for (const char* path : paths) {
            if (auto value = bz::data::ConfigValueUInt16(path)) {
                if (*value > 0) {
                    return *value;
                }
                spdlog::warn("Client startup config '{}' must be positive; falling back", path);
                return defaultValue;
            }
        }
        return defaultValue;
    };

    const uint16_t configWidth = readUInt16Config({"graphics.resolution.Width"}, 1280);
    const uint16_t configHeight = readUInt16Config({"graphics.resolution.Height"}, 720);
    const bool fullscreenEnabled = readBoolConfig({"graphics.Fullscreen"}, false);
    const bool vsyncEnabled = readBoolConfig({"graphics.VSync"}, true);

    const ClientCLIOptions cliOptions = ParseClientCLIOptions(argc, argv);

    const std::string clientUserConfigPath = clientUserConfigPathFs.string();
    ClientConfig clientConfig = ClientConfig::Load("");

    const std::string initialWorldDir = (cliOptions.worldExplicit && !cliOptions.worldDir.empty())
        ? cliOptions.worldDir
        : bz::data::Resolve("client-test").string();

    spdlog::trace("GLFW initialized successfully");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    

    FullscreenState fullscreenState;
    fullscreenState.windowedWidth = configWidth;
    fullscreenState.windowedHeight = configHeight;

    GLFWwindow *window = glfwCreateWindow(configWidth, configHeight, "BZFlag v3", nullptr, nullptr);
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
    glfwSwapInterval(vsyncEnabled ? 1 : 0);


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

    if (fullscreenEnabled) {
        ToggleFullscreen(window, fullscreenState, vsyncEnabled);
    }

    std::unique_ptr<Game> game;
    ServerConnector serverConnector(engine, cliOptions.playerName, initialWorldDir, game);
    ServerBrowserController serverBrowser(
        engine,
        clientConfig,
        clientUserConfigPath,
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
            ToggleFullscreen(window, fullscreenState, vsyncEnabled);
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