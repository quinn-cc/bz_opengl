#include "spdlog/spdlog.h"
#include "engine/server_engine.hpp"
#include "game.hpp"
#include "plugin.hpp"
#include "server/server_discovery.hpp"
#include "server/terminal_commands.hpp"
#include "server/server_cli_options.hpp"
#include <pybind11/embed.h>
#include <csignal>
#include <atomic>
#include <poll.h>
#include <unistd.h>
#include <filesystem>

#define MIN_FRAME_HZ (1.0f / 120.0f)

Game *g_game = nullptr;
ServerEngine *g_engine = nullptr;
std::atomic<bool> g_running{true};
namespace py = pybind11;

/**
 * Signal handler for graceful shutdown.
 *
 * @param signum The signal number.
 * 
 */
void signalHandler(int signum) {
    spdlog::info("Interrupt signal ({}) received. Shutting down...", signum);
    g_running = false;
}

int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::trace);

    // Register signal handler
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    ServerCLIOptions cliOptions = ParseServerCLIOptions(argc, argv);
    std::string worldDir = cliOptions.worldDir;

    // Get directory path for worldDir/config.json (using the json library)
    std::string configPath = worldDir + "/config.json";

    // Make sure configPath exists
    if (!std::filesystem::exists(configPath)) {
        spdlog::error("Config file not found: {}", configPath);
        return 1;
    }

    std::ifstream configFile(configPath);
    if (!configFile) {
        spdlog::error("main: Failed to open config file: {}", configPath);
        return 1;
    }

    nlohmann::json configJson;
    try {
        configFile >> configJson;
    } catch (const std::exception &e) {
        spdlog::error("main: Failed to parse config JSON: {}", e.what());
        return 1;
    }

    uint16_t port = cliOptions.hostPort;
    if (configJson.contains("hostPort") && configJson["hostPort"].is_number_unsigned()) {
        port = configJson["hostPort"].get<uint16_t>();
    }
    if (cliOptions.hostPortExplicit) {
        port = cliOptions.hostPort;
    }

    std::string serverName = "BZ OpenGL Server";
    if (configJson.contains("serverName") && configJson["serverName"].is_string()) {
        serverName = configJson["serverName"].get<std::string>();
    }

    ServerEngine engine(port);
    g_engine = &engine;
    spdlog::trace("ServerEngine initialized successfully");

    Game game(engine, serverName, configJson["settings"], worldDir + "/world");
    g_game = &game;
    spdlog::trace("Game initialized successfully");

    ServerDiscoveryBeacon discoveryBeacon(port, "BZ Server", worldDir);

    spdlog::trace("Loading plugins...");
    py::scoped_interpreter guard{};
    PluginAPI::loadPythonPlugins(configJson);
    spdlog::trace("Plugins loaded successfully");

    TimeUtils::time lastFrameTime = TimeUtils::GetCurrentTime();
    spdlog::trace("Starting main loop");

    std::string inputBuffer;
    struct pollfd pfd = { STDIN_FILENO, POLLIN, 0 };
    std::cout << "> " << std::flush;

    while (g_running) {
        TimeUtils::time currTime = TimeUtils::GetCurrentTime();  
        TimeUtils::duration deltaTime = TimeUtils::GetElapsedTime(lastFrameTime, currTime);
        
        if (deltaTime < MIN_FRAME_HZ) {
            TimeUtils::sleep(MIN_FRAME_HZ - deltaTime);
            continue;
        }

        lastFrameTime = currTime;

        // Non-blocking check for stdin input
        if (poll(&pfd, 1, 0) > 0 && (pfd.revents & POLLIN)) {
            std::string line;
            if (std::getline(std::cin, line)) {
                if (!line.empty()) {
                    std::string response = processTerminalInput(line);

                    if (!response.empty()) {
                        std::cout << response << std::endl;
                    }
                }
                std::cout << "> " << std::flush;
            }
        }

        engine.earlyUpdate(deltaTime);
        game.update(deltaTime);
        engine.lateUpdate(deltaTime);
    }

    spdlog::info("Server shutdown complete");
    return 0;
}