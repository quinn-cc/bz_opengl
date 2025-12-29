#pragma once
#include "engine/types.hpp"
#include <vector>
#include <memory>

namespace PluginAPI {
    std::vector<std::string> tokenizeMessage(const std::string &message);

    void sendChatMessage(client_id fromId, client_id toId, const std::string &text);
    void setWorldSetting(const std::string &key, float value);
    void killPlayer(client_id targetId);
    client_id getPlayerByName(const std::string &name);
    std::vector<client_id> getAllPlayerIds();

    std::string getPlayerName(client_id id);
    std::string getPlayerIP(client_id id);
}

class Plugin {
public:
    virtual ~Plugin() = default;
    virtual bool event_Chat(const ClientMsg_Chat &msg) = 0;
};

class PluginRegistry {
private:
    static std::vector<std::unique_ptr<Plugin>> plugins;

public:
    static inline void registerPlugin(std::unique_ptr<Plugin> plugin) {
        plugins.push_back(std::move(plugin));
    }

    static inline std::vector<std::unique_ptr<Plugin>>& getPlugins() {
        return plugins;
    }

    template <typename T> static inline bool handleEvent(const T &event) {
        // Assert that T is a subclass of ClientMsg
        static_assert(std::is_base_of_v<ClientMsg, T>, "T must be a subclass of ClientMsg");
        bool ret = false;

        for (const auto &plugin : plugins) {
            if constexpr (std::is_same_v<T, ClientMsg_Chat>) {
                if (plugin->event_Chat(event)) {
                    ret = true;
                }
            }
        }

        return ret;
    }
};

#ifndef REGISTER_PLUGIN
#define REGISTER_PLUGIN(ClassName) \
    namespace { \
        class ClassName##_Registrar { \
        public: \
            ClassName##_Registrar() { \
                PluginRegistry::registerPlugin(std::make_unique<ClassName>()); \
            } \
        }; \
        static ClassName##_Registrar ClassName##_instance; \
    }
#endif