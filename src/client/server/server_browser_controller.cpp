#include "client/server/server_browser_controller.hpp"

#include <unordered_set>
#include <utility>

#include "client/server/server_connector.hpp"

ServerBrowserController::ServerBrowserController(ClientEngine &engine,
                                                 const ClientConfig &clientConfig,
                                                 const std::string &defaultHost,
                                                 uint16_t defaultPort,
                                                 ServerConnector &connector)
    : engine(engine), connector(connector) {
    engine.gui->showServerBrowser({}, defaultHost, defaultPort);

    if (!clientConfig.serverLists.empty()) {
        serverListFetcher = std::make_unique<ServerListFetcher>(clientConfig.serverLists);
        serverListFetcher->requestRefresh();
        engine.gui->setServerBrowserStatus("Searching LAN and public server lists...", false);
    } else {
        engine.gui->setServerBrowserStatus("Searching local network for servers...", false);
    }
    discovery.startScan();
    engine.gui->setServerBrowserScanning(true);
    nextAutoScanTime = SteadyClock::now() + autoScanInterval;
}

void ServerBrowserController::triggerFullRefresh() {
    discovery.startScan();
    if (serverListFetcher) {
        serverListFetcher->requestRefresh();
    }
    engine.gui->setServerBrowserScanning(true);

    const auto &servers = discovery.getServers();
    if (servers.empty() && cachedRemoteServers.empty()) {
        if (serverListFetcher) {
            engine.gui->setServerBrowserStatus("Searching LAN and public server lists...", false);
        } else {
            engine.gui->setServerBrowserStatus("Searching local network for servers...", false);
        }
    } else if (servers.empty()) {
        engine.gui->setServerBrowserStatus("Searching local network for servers...", false);
    }
}

void ServerBrowserController::rebuildEntries() {
    const auto &servers = discovery.getServers();

    auto makeKey = [](const std::string &host, uint16_t port) {
        return host + ":" + std::to_string(port);
    };

    auto buildRemoteDescription = [](const ServerListFetcher::ServerRecord &record) {
        std::string description = record.sourceName.empty() ? "Public list" : record.sourceName;
        std::string details;
        if (record.activePlayers >= 0) {
            details = std::to_string(record.activePlayers);
            if (record.maxPlayers >= 0) {
                details += "/" + std::to_string(record.maxPlayers);
            }
            details += " players";
        }
        if (!record.gameMode.empty()) {
            if (!details.empty()) {
                details += " · ";
            }
            details += record.gameMode;
        }
        if (!details.empty()) {
            if (!description.empty()) {
                description += " — ";
            }
            description += details;
        }
        return description;
    };

    std::vector<GUI::ServerBrowserEntry> entries;
    entries.reserve(servers.size() + cachedRemoteServers.size());
    std::unordered_set<std::string> seen;
    seen.reserve(entries.capacity() > 0 ? entries.capacity() : 1);

    for (const auto &serverInfo : servers) {
        if (serverInfo.host.empty()) {
            continue;
        }
        auto key = makeKey(serverInfo.host, serverInfo.port);
        if (!seen.insert(key).second) {
            continue;
        }
        GUI::ServerBrowserEntry entry;
        entry.label = serverInfo.name.empty() ? "LAN server" : serverInfo.name;
        entry.host = serverInfo.host;
        entry.port = serverInfo.port;
        entry.description = serverInfo.world.empty() ? "Discovered via broadcast" : serverInfo.world;
        entry.displayHost = serverInfo.displayHost.empty() ? serverInfo.host : serverInfo.displayHost;
        entry.longDescription = serverInfo.world.empty()
            ? std::string("Discovered via LAN broadcast.")
            : (std::string("World: ") + serverInfo.world);
        entry.flags.clear();
        entry.activePlayers = -1;
        entry.maxPlayers = -1;
        entry.gameMode.clear();
        entries.push_back(std::move(entry));
    }

    for (const auto &record : cachedRemoteServers) {
        if (record.host.empty()) {
            continue;
        }
        uint16_t recordPort = record.port == 0 ? 1234 : record.port;
        auto key = makeKey(record.host, recordPort);
        if (!seen.insert(key).second) {
            continue;
        }
        GUI::ServerBrowserEntry entry;
        entry.label = record.name.empty() ? record.host : record.name;
        entry.host = record.host;
        entry.port = recordPort;
        entry.description = buildRemoteDescription(record);
        entry.displayHost = record.host;
        entry.longDescription = record.description.empty() ? entry.description : record.description;
        entry.flags = record.flags;
        entry.activePlayers = record.activePlayers;
        entry.maxPlayers = record.maxPlayers;
        entry.gameMode = record.gameMode;
        entries.push_back(std::move(entry));
    }

    engine.gui->setServerBrowserEntries(entries);
    if (!entries.empty()) {
        engine.gui->setServerBrowserStatus("Select a server to connect.", false);
    }
}

void ServerBrowserController::update() {
    auto nowSteady = SteadyClock::now();

    if (engine.gui->consumeServerBrowserRefreshRequest()) {
        triggerFullRefresh();
        nextAutoScanTime = nowSteady + autoScanInterval;
    } else if (!discovery.isScanning() && nowSteady >= nextAutoScanTime) {
        triggerFullRefresh();
        nextAutoScanTime = nowSteady + autoScanInterval;
    }

    discovery.update();
    bool remoteFetchingActive = serverListFetcher && serverListFetcher->isFetching();
    engine.gui->setServerBrowserScanning(discovery.isScanning() || remoteFetchingActive);

    bool entriesDirty = false;
    auto discoveryVersion = discovery.getGeneration();
    if (discoveryVersion != lastDiscoveryVersion) {
        lastDiscoveryVersion = discoveryVersion;
        entriesDirty = true;
    }

    if (serverListFetcher) {
        std::size_t remoteGeneration = serverListFetcher->getGeneration();
        if (remoteGeneration != lastServerListGeneration) {
            cachedRemoteServers = serverListFetcher->getServers();
            lastServerListGeneration = remoteGeneration;
            entriesDirty = true;
        }
    }

    if (entriesDirty) {
        rebuildEntries();
    }

    const auto &servers = discovery.getServers();
    bool lanEmpty = servers.empty();
    bool remoteEmpty = cachedRemoteServers.empty();

    if (auto selection = engine.gui->consumeServerBrowserSelection()) {
        connector.connect(selection->host, selection->port);
    }

    if (lanEmpty && remoteEmpty) {
        if (discovery.isScanning() || remoteFetchingActive) {
            engine.gui->setServerBrowserStatus("Searching LAN and public server lists...", false);
        } else {
            engine.gui->setServerBrowserStatus("No servers found. Start one locally or verify your Server List config.", true);
        }
    }
}
