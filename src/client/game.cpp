#include "game.hpp"
#include "spdlog/spdlog.h"

Game::Game(ClientEngine &engine, std::string playerName) : engine(engine) {
    player = new Player(*this, playerName); 
    spdlog::trace("Game: Player created successfully");
    world = new World(*this);
    spdlog::trace("Game: World created successfully");
};

Game::~Game() {
    delete player;
    spdlog::trace("Game: Player destroyed successfully");
    delete world;
    spdlog::trace("Game: World destroyed successfully");
}

void Game::update(TimeUtils::duration deltaTime) {
    player->update();
}