#include "spdlog/spdlog.h"
#include "engine/server_engine.hpp"
#include "game.hpp"
#include "cxxopts.hpp"
#include "plugin.hpp"
#include <pybind11/embed.h>
#include <csignal>
#include <atomic>
#include <poll.h>
#include <unistd.h>
#include <sstream>

#define MIN_FRAME_HZ (1.0f / 120.0f)

Game *g_game = nullptr;
ServerEngine *g_engine = nullptr;
std::atomic<bool> g_running{true};
std::vector<std::string> g_loadedPlugins;

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

void loadPythonPlugins() {
    namespace py = pybind11;

    py::module_ sys  = py::module_::import("sys");
    py::module_ glob = py::module_::import("glob");

    sys.attr("path").attr("insert")(0, "./");           // project root
    sys.attr("path").attr("insert")(0, "./plugins");    // plugin files
    sys.attr("path").attr("insert")(0, "./python");     // optional shared python code

    py::list files = glob.attr("glob")("../plugins/*.py");

    for (auto file : files) {
        try {
            py::print("[PY] Loading plugin:", file);
            py::eval_file(py::cast<std::string>(file), py::globals());
            g_loadedPlugins.push_back(py::cast<std::string>(file));
        } catch (py::error_already_set& e) {
            py::print("[PY ERROR]", e.what());
        }
    }
}

std::vector<std::string> splitString(const std::string &s) {
    std::istringstream iss(s);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string processTerminalInput(const std::string &input) {
    std::vector<std::string> args = splitString(input);
    if (args.empty()) return "";

    std::string cmd = args[0];

    if (cmd == "quit" || cmd == "exit") {
        g_running = false;
        return "Shutting down server...";
    } else if (cmd == "listPlugins") {
        std::string response = "Loaded Plugins:";
        for (const auto &plugin : g_loadedPlugins) {
            response += "\n - " + plugin;
        }
        return response;
    } else if (cmd == "manifest") {
        try {
            return g_game->world->getManifest().dump(4);
        } catch (const std::exception &e) {
            return std::string("Error retrieving manifest: ") + e.what();
        }
    } else if (cmd == "getAssetPath") {
        if (args.size() < 2) {
            return "Usage: getAssetPath <assetName>";
        }
        try {
            return g_game->world->getAssetPath(args[1]);
        } catch (const std::exception &e) {
            return std::string("Error: ") + e.what();
        }
    } else if (cmd == "defaultPlayerParameters") {
        std::string response = "Default Player Parameters:";
        for (const auto& [key, val] : g_game->world->getDefaultPlayerParameters()) {
            response += "\n - " + key + ": " + std::to_string(val);
        }
        return response;
    } else if (cmd == "listPlayers") {
        std::string response = "Connected Players:";
        for (const Client* client : g_game->getClients()) {
            response += "\n - ID: " + std::to_string(client->getId()) +
                        ", Name: " + client->getName() +
                        ", IP: " + client->getIP();
        }
        return response;
    } else {
        return "Unknown command: " + input;
    }
}

int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::trace);

    // Register signal handler
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    cxxopts::Options options("BZ", "This is the server.");
    options.add_options()
        ("w,world", "World directory", cxxopts::value<std::string>()->default_value("test-world/"));
    auto result = options.parse(argc, argv);
    std::string worldDir = result["world"].as<std::string>();

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

    uint16_t port = 1234;
    if (configJson.contains("hostPort") && configJson["hostPort"].is_number_unsigned()) {
        port = configJson["hostPort"].get<uint16_t>();
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

    spdlog::trace("Loading plugins...");
    py::scoped_interpreter guard{};
    loadPythonPlugins();
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