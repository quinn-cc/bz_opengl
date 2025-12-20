#include "spdlog/spdlog.h"
#include "engine/server_engine.hpp"
#include "game.hpp"
#include "cxxopts.hpp"

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
        deltaTime = std::max(deltaTime, 0.0001f);
        lastFrameTime = currTime;

        engine.update(deltaTime);
        game.update(deltaTime);
    }

    return 0;
}