#include "spdlog/spdlog.h"
#include "engine/server_engine.hpp"
#include "game.hpp"
#include "cxxopts.hpp"

#define MIN_FRAME_HZ (1.0f / 120.0f)

int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::trace);

    cxxopts::Options options("BZ", "This is the server.");
    options.add_options()
        ("p,port", "Listening port", cxxopts::value<uint16_t>()->default_value("1234"));
    auto result = options.parse(argc, argv);
    uint16_t listenPort = result["port"].as<uint16_t>();

    ServerEngine engine(listenPort);
    spdlog::trace("ServerEngine initialized successfully");

    Game game(engine);
    spdlog::trace("Game initialized successfully");

    TimeUtils::time lastFrameTime = TimeUtils::GetCurrentTime();
    spdlog::trace("Starting main loop");

    while (true) {
        TimeUtils::time currTime = TimeUtils::GetCurrentTime();  
        TimeUtils::duration deltaTime = TimeUtils::GetElapsedTime(lastFrameTime, currTime);
        
        if (deltaTime < MIN_FRAME_HZ) {
            continue;
        }

        lastFrameTime = currTime;

        engine.earlyUpdate(deltaTime);
        game.update(deltaTime);
        engine.lateUpdate(deltaTime);
    }

    return 0;
}