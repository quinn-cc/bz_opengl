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

void Game::earlyUpdate(TimeUtils::duration deltaTime) {
    world->update();

    if (!world->isInitialized()) {
        return;
    }

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

    player->earlyUpdate();

    if (auto msg = engine.network->peekMessage<ServerMsg_Connection>()) {
        Client *client = new Client(*this, msg->clientId);
        clients.push_back(client);
        spdlog::trace("Game: New client connected with ID {}", msg->clientId);
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
    }

    // Listen for incoming shots (they will always be global ids)
    if (auto msg = engine.network->peekMessage<ServerMsg_CreateShot>()) {
        Shot *newShot = new Shot(
            *this,
            msg->globalShotId,
            msg->position,
            msg->velocity
        );
        shots.push_back(newShot);
    }

    // Listen for remove shot messages
    if (auto msg = engine.network->peekMessage<ServerMsg_RemoveShot>()) {
        for (Shot *shot : shots) {
            if (shot->isEqual(msg->shotId, msg->isGlobalId)) {
                shots.erase(
                    std::remove(shots.begin(), shots.end(), shot),
                    shots.end()
                );
                delete shot;
                break;
            }
        }
    }
}

void Game::lateUpdate(TimeUtils::duration deltaTime) {
    if (!world->isInitialized()) {
        return;
    }

    player->lateUpdate();

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

Client *Game::getClientById(client_id id) {
    for (Client *client : clients) {
        if (client->isEqual(id)) {
            return client;
        }
    }
    return nullptr;
}