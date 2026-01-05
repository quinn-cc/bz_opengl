#include "chat.hpp"
#include "game.hpp"
#include <string>
#include <sstream>
#include "spdlog/spdlog.h"
#include "engine/server_engine.hpp"
#include "plugin.hpp"

Chat::Chat(Game &game) : game(game) {

}

Chat::~Chat() {
    messages.clear();
}

void Chat::update() {
    // Listen for incoming chat
    if (auto *chatMsg = game.engine.network->peekMessage<ClientMsg_Chat>()) {
        // Info out the chat message
        spdlog::info("Client: {}, Message: {}", game.getClient(chatMsg->clientId)->getName(), chatMsg->text);
        
        std::string message = std::string(chatMsg->text);
        messages.push_back(message);

        bool handled = g_runPluginCallbacks<ClientMsg_Chat>(*chatMsg);

        if (!handled) {
            ServerMsg_Chat serverChatMsg;
            serverChatMsg.fromId = chatMsg->clientId;
            serverChatMsg.toId = chatMsg->toId;
            serverChatMsg.text = chatMsg->text;

            if (chatMsg->toId == BROADCAST_CLIENT_ID) {
                game.engine.network->sendExcept<ServerMsg_Chat>(chatMsg->clientId, &serverChatMsg);
            } else {
                game.engine.network->send<ServerMsg_Chat>(chatMsg->toId, &serverChatMsg);
            }
        }
    }
}

