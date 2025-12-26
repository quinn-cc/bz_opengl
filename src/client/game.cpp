#include "game.hpp"
#include "spdlog/spdlog.h"

Game::Game(ClientEngine &engine, std::string playerName) : engine(engine) {
    player = new Player(*this, playerName); 
    spdlog::trace("Game: Player created successfully");
    world = new World(*this);
    spdlog::trace("Game: World created successfully");
    console = new Console(*this);
    spdlog::trace("Game: Console created successfully");

    focusState = FOCUS_STATE_GAME;
};

Game::~Game() {
    delete player;
    spdlog::trace("Game: Player destroyed successfully");
    delete world;
    spdlog::trace("Game: World destroyed successfully");
    delete console;
    spdlog::trace("Game: Console destroyed successfully");
}

void Game::update(TimeUtils::duration deltaTime) {
    if (focusState == FOCUS_STATE_GAME && engine.input->getInputState().chat) {
        focusState = FOCUS_STATE_CONSOLE;
        spdlog::trace("Game: Switching focus to console");
        console->focusChatInput();
    }

    console->update();

    if (focusState == FOCUS_STATE_CONSOLE && !console->isChatInFocus()) {
        focusState = FOCUS_STATE_GAME;
        spdlog::trace("Game: Returning focus to game");
    }

    player->update();

    if (auto msg = engine.network->peekMessage<ServerMsg_Connection>()) {
        Client *client = new Client(*this, msg->clientId);
        clients.push_back(client);
        spdlog::trace("Game: New client connected with ID {}", msg->clientId);

        engine.network->popMessage(msg);
    }

    if (auto msg = engine.network->peekMessage<ServerMsg_Disconnection>()) {
        auto it = std::find_if(clients.begin(), clients.end(),
            [msg](Client *client) {
                return client->isEqual(msg->clientId);
            }
        );

        if (it != clients.end()) {
            delete *it;
            clients.erase(it);
            spdlog::trace("Game: Client disconnected with ID {}", msg->clientId);
        }

        engine.network->popMessage(msg);
    }


    if (focusState == FOCUS_STATE_GAME) {
        if (engine.input->getInputState().fire) {
            glm::vec3 position = player->getPosition();
            glm::vec3 velocity = player->getVelocity();
            glm::vec3 forward = player->getForwardVector();

            glm::vec3 shotPosition = position + forward * 2.0f;
            glm::vec3 shotVelocity = forward * world->getSettings().shotSpeed + velocity;

            Shot *shot = new Shot(*this, getNextLocalShotId(), 0, shotPosition, shotVelocity);
            shots.push_back(shot);
        }
    }

    for (Client *client : clients) {
        client->update();
    }

    for (Shot *shot : shots) {
        shot->update(deltaTime);
    }

    std::vector<std::string> scoreboardNames;
    for (Client *client : clients) {
        scoreboardNames.push_back(client->getName());
    }
    scoreboardNames.push_back(player->getName());
    engine.gui->setScoreboardPlayerNames(scoreboardNames);
}