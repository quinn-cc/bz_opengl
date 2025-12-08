#include "client.hpp"
#include <string.h>
#include <vector>
#include <algorithm>
#include <spdlog/spdlog.h>

std::string Client::ToString() {
    std::string out = "";
    out += "Client{name=";
    out += name;
    out += ",";
    out += "clientId=";
    out += std::to_string(id);
    out += "}";
    return out;
}
