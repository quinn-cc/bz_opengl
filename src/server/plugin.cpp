#include "plugin.hpp"
#include "game.hpp"
#include "engine/server_engine.hpp"

extern Game* g_game;
extern ServerEngine* g_engine;

std::vector<std::unique_ptr<Plugin>> PluginRegistry::plugins;

std::vector<std::string> PluginAPI::tokenizeMessage(const std::string &message) {
    std::vector<std::string> tokens;
    std::istringstream iss(message);
    std::string token;
    
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

void PluginAPI::sendChatMessage(client_id fromId, client_id toId, const std::string &text) {
    ServerMsg_Chat serverChatMsg;
    serverChatMsg.fromId = fromId;
    serverChatMsg.toId = toId;
    strcpy(serverChatMsg.text, text.c_str());
    g_engine->network->send<ServerMsg_Chat>(toId, serverChatMsg);
}

void PluginAPI::setWorldSetting(const std::string &key, float value) {
    g_game->world->setSetting(key, value);
}

void PluginAPI::killPlayer(client_id targetId) {
    Client *client = g_game->getClient(targetId);

    if (client) {
        client->die();
    }
}

client_id PluginAPI::getPlayerByName(const std::string &name) {
    return g_game->getClientByName(name)->getId();
}

std::vector<client_id> PluginAPI::getAllPlayerIds() {
    std::vector<client_id> ids;
    for (const Client* client : g_game->getClients()) {
        ids.push_back(client->getId());
    }
    return ids;
}

std::string PluginAPI::getPlayerName(client_id id) {
    Client* client = g_game->getClient(id);
    if (client) {
        return client->getName();
    }
    return "";
}

std::string PluginAPI::getPlayerIP(client_id id) {
    Client* client = g_game->getClient(id);
    if (client) {
        return client->getIP();
    }
    return "";
}