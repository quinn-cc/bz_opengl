#include "client/client_cli_options.hpp"

#include "common/data_path_resolver.hpp"
#include "cxxopts.hpp"

namespace {

std::string ConfiguredPortDefault() {
    if (const auto *portNode = bz::data::ConfigValue("network.ServerPort")) {
        if (portNode->is_string()) {
            return portNode->get<std::string>();
        }
        if (portNode->is_number_unsigned()) {
            return std::to_string(portNode->get<unsigned int>());
        }
    }
    return std::string("0");
}

} // namespace

ClientCLIOptions ParseClientCLIOptions(int argc, char *argv[]) {
    cxxopts::Options options("BZ", "This is the client.");
    options.add_options()
        ("n,name", "Player name", cxxopts::value<std::string>()->default_value("Player"));
    options.add_options()
        ("a,addr", "Connection address", cxxopts::value<std::string>()->default_value("localhost"));
    options.add_options()
        ("p,port", "Connection port", cxxopts::value<uint16_t>()->default_value(ConfiguredPortDefault()));
    options.add_options()
        ("w,world", "World directory", cxxopts::value<std::string>()->default_value(""));

    auto result = options.parse(argc, argv);

    ClientCLIOptions parsed;
    parsed.playerName = result["name"].as<std::string>();
    parsed.connectAddr = result["addr"].as<std::string>();
    parsed.connectPort = result["port"].as<uint16_t>();
    parsed.worldDir = result["world"].as<std::string>();
    parsed.addrExplicit = result.count("addr") > 0;
    parsed.worldExplicit = result.count("world") > 0;
    return parsed;
}
