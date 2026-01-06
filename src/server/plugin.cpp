#include "plugin.hpp"
#include "game.hpp"
#include "engine/server_engine.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/embed.h>

extern Game* g_game;
extern ServerEngine* g_engine;
std::map<ClientMsg_Type, std::vector<pybind11::function>> g_pluginCallbacks;

void PluginAPI::registerCallback(ClientMsg_Type type, pybind11::function func) {
    if (g_pluginCallbacks.find(type) == g_pluginCallbacks.end()) {
        g_pluginCallbacks[type] = std::vector<pybind11::function>();
    }
    g_pluginCallbacks[type].push_back(func);
}

void PluginAPI::sendChatMessage(client_id fromId, client_id toId, const std::string &text) {
    ServerMsg_Chat serverChatMsg;
    serverChatMsg.fromId = fromId;
    serverChatMsg.toId = toId;
    serverChatMsg.text = text;
    g_engine->network->send<ServerMsg_Chat>(toId, &serverChatMsg);
}

void PluginAPI::setWorldSetting(const std::string &key, float value) {
    //g_game->world->setSetting(key, value);
}

void PluginAPI::killPlayer(client_id targetId) {
    Client *client = g_game->getClient(targetId);

    if (client) {
        client->die();
    }
}

client_id PluginAPI::getPlayerByName(const std::string &name) {
    Client* client = g_game->getClientByName(name);
    if (client) {
        return client->getId();
    }
    return 0; // Return invalid ID if player not found
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

PYBIND11_EMBEDDED_MODULE(bz_plugins, m) {
    m.doc() = "Plugin API for BZ OpenGL server plugins";

    pybind11::enum_<ClientMsg_Type>(m, "event_type")
        .value("CHAT", ClientMsg_Type::ClientMsg_Type_CHAT)
        .value("PLAYER_JOIN", ClientMsg_Type::ClientMsg_Type_PLAYER_JOIN)
        .value("PLAYER_LEAVE", ClientMsg_Type::ClientMsg_Type_PLAYER_LEAVE)
        .export_values();

    // Callback registration function
    m.def("register_callback", &PluginAPI::registerCallback, "Register a callback",
          pybind11::arg("type"), pybind11::arg("callback"));

    m.def("send_chat_message", &PluginAPI::sendChatMessage, "Send a chat message",
          pybind11::arg("from_id"), pybind11::arg("to_id"), pybind11::arg("text"));
    m.def("set_world_setting", &PluginAPI::setWorldSetting, "Set a world setting",
          pybind11::arg("key"), pybind11::arg("value"));
    m.def("kill_player", &PluginAPI::killPlayer, "Kill a player",
          pybind11::arg("target_id"));
    m.def("get_player_by_name", &PluginAPI::getPlayerByName, "Get a player ID by name",
          pybind11::arg("name"));
    m.def("get_all_player_ids", &PluginAPI::getAllPlayerIds, "Get all player IDs");
    m.def("get_player_name", &PluginAPI::getPlayerName, "Get a player's name by ID",
          pybind11::arg("id"));
    m.def("get_player_ip", &PluginAPI::getPlayerIP, "Get a player's IP by ID",
          pybind11::arg("id"));
}