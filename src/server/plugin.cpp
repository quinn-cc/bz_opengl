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

bool PluginAPI::setPlayerParameter(client_id playerId, const std::string &param, const pybind11::object &value) {
    Client* client = g_game->getClient(playerId);
    if (client) {
        return client->setParameter(param, value.cast<float>());
    }
    return false;
}

void PluginAPI::killPlayer(client_id targetId) {
    Client *client = g_game->getClient(targetId);

    if (client) {
        client->die();
    }
}

std::optional<client_id> PluginAPI::getPlayerByName(const std::string &name) {
    Client* client = g_game->getClientByName(name);
    if (client) {
        return client->getId();
    }
    return std::nullopt;
}

std::vector<client_id> PluginAPI::getAllPlayerIds() {
    std::vector<client_id> ids;
    for (const Client* client : g_game->getClients()) {
        ids.push_back(client->getId());
    }
    return ids;
}

std::optional<std::string> PluginAPI::getPlayerName(client_id id) {
    Client* client = g_game->getClient(id);
    if (client) {
        return client->getName();
    }
    return std::nullopt;
}

std::optional<std::string> PluginAPI::getPlayerIP(client_id id) {
    Client* client = g_game->getClient(id);
    if (client) {
        return client->getIP();
    }
    return std::nullopt;
}

PYBIND11_EMBEDDED_MODULE(bzapi, m) {
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
    m.def("set_player_parameter", &PluginAPI::setPlayerParameter, "Set a player parameter",
          pybind11::arg("player_id"), pybind11::arg("param"), pybind11::arg("value"));
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