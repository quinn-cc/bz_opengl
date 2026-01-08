#include "server/server_cli_options.hpp"

#include "cxxopts.hpp"
#include "common/data_path_resolver.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>

ServerCLIOptions ParseServerCLIOptions(int argc, char *argv[]) {
    cxxopts::Options options("BZ", "This is the server.");
    options.add_options()
        ("w,world", "World directory", cxxopts::value<std::string>())
        ("D,default-world", "Use bundled default world")
        ("p,port", "Server listen port", cxxopts::value<uint16_t>()->default_value("1234"));

    auto result = options.parse(argc, argv);

    ServerCLIOptions parsed;
    if (result.count("world") && result.count("default-world")) {
        throw std::runtime_error("Cannot specify both -w/--world and -D/--default-world");
    }

    if (result.count("default-world")) {
        parsed.worldSpecified = true;

        const auto serverConfigPath = bz::data::Resolve("server/config.json");
        auto serverConfigOpt = bz::data::LoadJsonFile(serverConfigPath, "data/server/config.json", spdlog::level::err);
        if (!serverConfigOpt || !serverConfigOpt->is_object()) {
            throw std::runtime_error("default world flag requires data/server/config.json to be a JSON object");
        }

        auto it = serverConfigOpt->find("defaultWorld");
        if (it == serverConfigOpt->end() || !it->is_string()) {
            throw std::runtime_error("defaultWorld missing or not a string in data/server/config.json");
        }

        parsed.worldDir = it->get<std::string>();
        parsed.customWorldProvided = false;
    }

    if (result.count("world")) {
        parsed.worldDir = result["world"].as<std::string>();
        parsed.worldSpecified = true;
        parsed.customWorldProvided = true;
    }

    parsed.hostPort = result["port"].as<uint16_t>();
    parsed.hostPortExplicit = result.count("port") > 0;
    return parsed;
}
