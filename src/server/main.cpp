#include "spdlog/spdlog.h"
#include "engine/server_engine.hpp"
#include "game.hpp"
#include "plugin.hpp"
#include "server/server_discovery.hpp"
#include "server/terminal_commands.hpp"
#include "server/server_cli_options.hpp"
#include "common/data_path_resolver.hpp"
#include <nlohmann/json.hpp>
#include <pybind11/embed.h>
#include <csignal>
#include <atomic>
#include <poll.h>
#include <unistd.h>
#include <filesystem>
#include <vector>

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

    ServerCLIOptions cliOptions;
    try {
        cliOptions = ParseServerCLIOptions(argc, argv);
    } catch (const std::exception &ex) {
        spdlog::error("Failed to parse server command line options: {}", ex.what());
        return 1;
    }

    if (!cliOptions.worldSpecified) {
        spdlog::error("No world directory specified. Use -w <directory> or -D to load the bundled default world.");
        return 1;
    }

    std::filesystem::path worldDirPath = bz::data::Resolve(cliOptions.worldDir);

    if (!std::filesystem::is_directory(worldDirPath)) {
        spdlog::error("World directory not found: {}", worldDirPath.string());
        return 1;
    }
    std::filesystem::path configPath = worldDirPath / "config.json";
    auto worldConfigOpt = bz::data::LoadJsonFile(configPath, "world config", spdlog::level::err);
    if (!worldConfigOpt) {
        spdlog::error("main: Failed to load world config: {}", configPath.string());
        return 1;
    }
    if (!worldConfigOpt->is_object()) {
        spdlog::error("main: World config '{}' is not a JSON object", configPath.string());
        return 1;
    }
    nlohmann::json worldConfig = std::move(*worldConfigOpt);

    nlohmann::json mergedConfig = nlohmann::json::object();
    const std::vector<bz::data::ConfigLayerSpec> serverConfigSpecs = {
        {"common/config.json", "data/common/config.json", spdlog::level::err, true},
        {"server/config.json", "data/server/config.json", spdlog::level::err, true}
    };

    const auto serverLayers = bz::data::LoadConfigLayers(serverConfigSpecs);
    if (serverLayers.size() != serverConfigSpecs.size()) {
        spdlog::error("main: Failed to load required server configuration layers");
        return 1;
    }
    for (const auto &layer : serverLayers) {
        bz::data::MergeJsonObjects(mergedConfig, layer.json);
    }

    bz::data::MergeJsonObjects(mergedConfig, worldConfig);

    uint16_t port = cliOptions.hostPort;
    if (mergedConfig.contains("hostPort") && mergedConfig["hostPort"].is_number_unsigned()) {
        port = mergedConfig["hostPort"].get<uint16_t>();
    }
    if (cliOptions.hostPortExplicit) {
        port = cliOptions.hostPort;
    }

    std::string serverName = "BZ OpenGL Server";
    if (mergedConfig.contains("serverName") && mergedConfig["serverName"].is_string()) {
        serverName = mergedConfig["serverName"].get<std::string>();
    }

    ServerEngine engine(port);
    g_engine = &engine;
    spdlog::trace("ServerEngine initialized successfully");

    const bool shouldZipWorld = cliOptions.customWorldProvided;

    Game game(engine, serverName, worldConfig, worldDirPath.string(), shouldZipWorld);
    g_game = &game;
    spdlog::trace("Game initialized successfully");

    ServerDiscoveryBeacon discoveryBeacon(port, "BZ Server", worldDirPath.filename().string());

    spdlog::trace("Loading plugins...");
    py::scoped_interpreter guard{};
    PluginAPI::loadPythonPlugins(mergedConfig);
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