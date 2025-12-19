#include "game.hpp"

Game::Game(ClientEngine &engine) : engine(engine) {
    player = new Player(*this);
    world = new World(*this);
};

Game::~Game() {
    delete player;
    delete world;
}

void Game::update(TimeUtils::duration deltaTime) {
    player->update();
}