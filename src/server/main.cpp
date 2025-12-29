#include "spdlog/spdlog.h"
#include "engine/server_engine.hpp"
#include "game.hpp"
#include "cxxopts.hpp"
#include "plugin.hpp"

#define MIN_FRAME_HZ (1.0f / 120.0f)

Game *g_game = nullptr;
ServerEngine *g_engine = nullptr;

int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::trace);

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

    TimeUtils::time lastFrameTime = TimeUtils::GetCurrentTime();
    spdlog::trace("Starting main loop");

    while (true) {
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

    return 0;
}