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
            strcpy(chatMsg.text, message.c_str());
            game.engine.network->send<ClientMsg_Chat>(chatMsg);
            game.engine.gui->clearChatInputBuffer();
        }

        if (!game.engine.gui->getChatInputFocus()) {
            chatInFocus = false;
        }
    }

    if (auto msg = game.engine.network->peekMessage<ServerMsg_Chat>()) {
        game.engine.gui->addConsoleLine(std::string(msg->name), std::string(msg->text));
        game.engine.network->popMessage(msg);
    }
}