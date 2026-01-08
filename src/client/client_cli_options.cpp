#include "client/client_cli_options.hpp"

#include "cxxopts.hpp"

ClientCLIOptions ParseClientCLIOptions(int argc, char *argv[]) {
    cxxopts::Options options("BZ", "This is the client.");
    options.add_options()
        ("n,name", "Player name", cxxopts::value<std::string>()->default_value("Player"));
    options.add_options()
        ("a,addr", "Connection address", cxxopts::value<std::string>()->default_value("localhost"));
    options.add_options()
        ("p,port", "Connection port", cxxopts::value<uint16_t>()->default_value("1234"));
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
