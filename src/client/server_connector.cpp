#include "client/server_connector.hpp"

#include <utility>

#include "engine/client_engine.hpp"
#include "game.hpp"
#include "spdlog/spdlog.h"

ServerConnector::ServerConnector(ClientEngine &engine,
                                 std::string playerName,
                                 std::string worldDir,
                                 std::unique_ptr<Game> &game)
    : engine(engine),
      game(game),
      playerName(std::move(playerName)),
      worldDir(std::move(worldDir)) {}

bool ServerConnector::connect(const std::string &targetHost, uint16_t targetPort) {
    std::string status = "Connecting to " + targetHost + ":" + std::to_string(targetPort) + "...";
    engine.gui->setServerBrowserStatus(status, false);
    spdlog::info("Attempting to connect to {}:{}", targetHost, targetPort);

    if (engine.network->connect(targetHost, targetPort, 50)) {
        spdlog::info("Connected to server at {}:{}", targetHost, targetPort);
        game = std::make_unique<Game>(engine, playerName, worldDir);
        spdlog::trace("Game initialized successfully");
        engine.gui->hideServerBrowser();
        return true;
    }

    spdlog::error("Failed to connect to server at {}:{}", targetHost, targetPort);
    std::string errorMsg = "Unable to reach " + targetHost + ":" + std::to_string(targetPort) + ".";
    engine.gui->setServerBrowserStatus(errorMsg, true);
    return false;
}
