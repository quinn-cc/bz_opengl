#pragma once

#include <cstdint>
#include <string>

struct ClientCLIOptions {
    std::string playerName;
    std::string connectAddr;
    uint16_t connectPort;
    std::string worldDir;
    bool addrExplicit = false;
};

ClientCLIOptions ParseClientCLIOptions(int argc, char *argv[]);
