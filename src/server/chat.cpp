#include "chat.hpp"
#include "game.hpp"
#include <string>
#include "spdlog/spdlog.h"
#include "engine/server_engine.hpp"

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

        if (message[0] == '/') {
            // If the first characters are "/set ", then its a seting change
            if (message.rfind("/set ", 0) == 0) {
                size_t firstSpace = message.find(' ', 5);
                if (firstSpace != std::string::npos) {
                    std::string key = message.substr(5, firstSpace - 5);
                    std::string valueStr = message.substr(firstSpace + 1);
                    try {
                        float value = std::stof(valueStr);
                        game.world->setSetting(key, value);
                        
                        spdlog::info("Client: {} changed setting: {} to {}", game.getClient(chatMsg->clientId)->getName(), key, value);
                    } catch (const std::exception &e) {
                        spdlog::error("Failed to change setting: {}", e.what());
                    }
                }
            }
        } else {
            ServerMsg_Chat serverChatMsg;
            strcpy(serverChatMsg.name, game.getClient(chatMsg->clientId)->getName().c_str());
            strcpy(serverChatMsg.text, chatMsg->text);
            game.engine.network->sendExcept<ServerMsg_Chat>(chatMsg->clientId, serverChatMsg);
        }
    }
}

