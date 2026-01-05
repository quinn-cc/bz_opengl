#include "console.hpp"
#include "game.hpp"
#include "spdlog/spdlog.h"

Console::Console(Game &game) : game(game) {

}

void Console::focusChatInput() {
    game.engine.gui->focusChatInput();
    chatInFocus = true;
}

void Console::update() {
    if (chatInFocus) {
        if (game.engine.gui->getChatInputBuffer().length() > 0) {
            spdlog::trace("Console::update: Processing submitted chat input");
            std::string message = game.engine.gui->getChatInputBuffer();

            game.engine.gui->addConsoleLine(game.player->getName(), message);

            ClientMsg_Chat chatMsg;
            chatMsg.toId = BROADCAST_CLIENT_ID;
            chatMsg.text = message;
            game.engine.network->send<ClientMsg_Chat>(chatMsg);
            game.engine.gui->clearChatInputBuffer();
        }

        if (!game.engine.gui->getChatInputFocus()) {
            chatInFocus = false;
        }
    }

    if (auto msg = game.engine.network->peekMessage<ServerMsg_Chat>()) {
        std::string name = "";

        if (game.getClientById(msg->fromId) == nullptr) {
            if (msg->fromId == game.player->getClientId()) {
                name = "YOU";
            } else if (msg->fromId == SERVER_CLIENT_ID) {
                name = "SERVER";
            } else {
                name = "UNKNOWN";
            }
        }

        if (msg->toId == game.player->getClientId()) {
            name = "[" + name + " -> YOU]";
        }

        game.engine.gui->addConsoleLine(name, msg->text);
    }
}