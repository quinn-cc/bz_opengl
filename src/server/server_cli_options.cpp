#include "server/server_cli_options.hpp"

#include "cxxopts.hpp"

ServerCLIOptions ParseServerCLIOptions(int argc, char *argv[]) {
    cxxopts::Options options("BZ", "This is the server.");
    options.add_options()
        ("w,world", "World directory", cxxopts::value<std::string>()->default_value("test-world/"));
    options.add_options()
        ("p,port", "Server listen port", cxxopts::value<uint16_t>()->default_value("1234"));

    auto result = options.parse(argc, argv);

    ServerCLIOptions parsed;
    parsed.worldDir = result["world"].as<std::string>();
    parsed.hostPort = result["port"].as<uint16_t>();
    parsed.hostPortExplicit = result.count("port") > 0;
    return parsed;
}
