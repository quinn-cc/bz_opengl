#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "client/config_client.hpp"
#include "client/server/server_discovery.hpp"
#include "client/server/server_list_fetcher.hpp"
#include "engine/client_engine.hpp"

class ServerConnector;

class ServerBrowserController {
public:
    ServerBrowserController(ClientEngine &engine,
                            const ClientConfig &clientConfig,
                            const std::string &defaultHost,
                            uint16_t defaultPort,
                            ServerConnector &connector);

    void update();

private:
    using SteadyClock = std::chrono::steady_clock;

    void triggerFullRefresh();
    void rebuildEntries();

    ClientEngine &engine;
    ServerConnector &connector;
    ServerDiscovery discovery;
    std::unique_ptr<ServerListFetcher> serverListFetcher;
    std::vector<ServerListFetcher::ServerRecord> cachedRemoteServers;

    SteadyClock::time_point nextAutoScanTime;
    const std::chrono::seconds autoScanInterval{5};
    std::size_t lastDiscoveryVersion = 0;
    std::size_t lastServerListGeneration = 0;
};
