#include "console.hpp"
#include "game.hpp"
#include "spdlog/spdlog.h"

Console::Console(Game &game) : game(game) {

}

void Console::update() {
    if (game.getFocusState() == FOCUS_STATE_CONSOLE) {
        game.engine.gui->setChatInputFocus(true);
        chatInFocus = true;
    } else if (game.getFocusState() == FOCUS_STATE_CONSOLE && game.engine.input->getInputState().escape) {
        game.engine.gui->setChatInputFocus(false);
        chatInFocus = false;
    }

    if (game.engine.gui->hasChatInputBuffer() && game.engine.gui->getChatInputBuffer().length() > 0) {
        std::string message = game.engine.gui->getChatInputBuffer();

        game.engine.gui->addConsoleLine(game.player->getName(), message);

        ClientMsg_Chat chatMsg;
        strcpy(chatMsg.text, message.c_str());
        game.engine.network->send<ClientMsg_Chat>(chatMsg);
        game.engine.gui->setChatInputFocus(false);
        chatInFocus = false;
        spdlog::trace("Console::update: Chat focus disabled");
    }

    if (auto msg = game.engine.network->peekMessage<ServerMsg_Chat>()) {
        game.engine.gui->addConsoleLine(std::string(msg->name), std::string(msg->text));
        game.engine.network->popMessage(msg);
    }
}