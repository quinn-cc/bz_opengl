#include "spdlog/spdlog.h"
#include "engine/server_engine.hpp"
#include "game.hpp"
#include "cxxopts.hpp"
#include "plugin.hpp"
#include <pybind11/embed.h>
#include <csignal>
#include <atomic>

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

void loadPythonPlugins() {
    namespace py = pybind11;

    py::module_ sys  = py::module_::import("sys");
    py::module_ glob = py::module_::import("glob");

    sys.attr("path").attr("insert")(0, "./");           // project root
    sys.attr("path").attr("insert")(0, "./plugins");    // plugin files
    sys.attr("path").attr("insert")(0, "./python");     // optional shared python code

    py::list files = glob.attr("glob")("./plugins/*.py");

    for (auto file : files) {
        try {
            py::print("[PY] Loading plugin:", file);
            py::eval_file(py::cast<std::string>(file), py::globals());
        } catch (py::error_already_set& e) {
            py::print("[PY ERROR]", e.what());
        }
    }
}

int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::trace);

    // Register signal handler
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    cxxopts::Options options("BZ", "This is the server.");
    options.add_options()
        ("w,world", "World directory", cxxopts::value<std::string>()->default_value("test-world/"))
        ("p,port", "Listening port", cxxopts::value<uint16_t>()->default_value("1234"));
    auto result = options.parse(argc, argv);
    uint16_t listenPort = result["port"].as<uint16_t>();
    std::string worldDir = result["world"].as<std::string>();

    ServerEngine engine(listenPort);
    g_engine = &engine;
    spdlog::trace("ServerEngine initialized successfully");

    Game game(engine, worldDir);
    g_game = &game;
    spdlog::trace("Game initialized successfully");

    spdlog::trace("Loading plugins...");
    py::scoped_interpreter guard{};
    loadPythonPlugins();
    spdlog::trace("Plugins loaded successfully");

    TimeUtils::time lastFrameTime = TimeUtils::GetCurrentTime();
    spdlog::trace("Starting main loop");

    while (g_running) {
        TimeUtils::time currTime = TimeUtils::GetCurrentTime();  
        TimeUtils::duration deltaTime = TimeUtils::GetElapsedTime(lastFrameTime, currTime);
        
        if (deltaTime < MIN_FRAME_HZ) {
            TimeUtils::sleep(MIN_FRAME_HZ - deltaTime);
            continue;
        }

        lastFrameTime = currTime;

        engine.earlyUpdate(deltaTime);
        game.update(deltaTime);
        engine.lateUpdate(deltaTime);
    }

    spdlog::info("Server shutdown complete");
    return 0;
}