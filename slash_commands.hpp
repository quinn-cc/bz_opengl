#include "plugin.hpp"

class SlashCommandsPlugin : public Plugin {
public:
    bool event_Chat(const ClientMsg_Chat &msg) override;
};