#include "slash_commands.hpp"


bool SlashCommandsPlugin::event_Chat(const ClientMsg_Chat &msg) {
    std::vector<std::string> tokens = PluginAPI::tokenizeMessage(std::string(msg.text));

    if (tokens.size() == 0) {
        return false;
    }

    const std::string &command = tokens[0];

    if (command[0] != '/') {
        return false;
    }

    if (command == "/set") {
        if (tokens.size() < 3) {
            PluginAPI::sendChatMessage(SERVER_CLIENT_ID, msg.clientId, "Usage: /set <setting> <value>");
            return true;
        }

        std::string key = tokens[1];
        float value = std::stof(tokens[2]);

        PluginAPI::setWorldSetting(key, value);
        PluginAPI::sendChatMessage(SERVER_CLIENT_ID, BROADCAST_CLIENT_ID, "Setting " + key + " set to " + std::to_string(value));
    } else if (command == "/kill") {
        if (tokens.size() < 2) {
            PluginAPI::sendChatMessage(SERVER_CLIENT_ID, msg.clientId, "Usage: /kill <playername>");
            return true;
        }

        std::string targetName = tokens[1];
        client_id targetId = PluginAPI::getPlayerByName(targetName);
        if (targetId != 0) {
            PluginAPI::killPlayer(targetId);
        }
    } else if (command == "/playerlist") {
        PluginAPI::sendChatMessage(SERVER_CLIENT_ID, msg.clientId, "Online Players:");
        std::vector<client_id> playerIds = PluginAPI::getAllPlayerIds();
        for (client_id pid : playerIds) {
            std::string playerName = PluginAPI::getPlayerName(pid);
            std::string line = "- " + playerName + " (ID: " + std::to_string(pid) + ")";
            line += " @ " + PluginAPI::getPlayerIP(pid);
            PluginAPI::sendChatMessage(SERVER_CLIENT_ID, msg.clientId, line);
        }
    } else {
        PluginAPI::sendChatMessage(SERVER_CLIENT_ID, msg.clientId, "Unknown command: " + command);
    }

    return true;
};


REGISTER_PLUGIN(SlashCommandsPlugin)