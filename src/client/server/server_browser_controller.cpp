#include "client/server/server_browser_controller.hpp"

#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "client/server/server_connector.hpp"

namespace {
std::string trimCopy(const std::string &value) {
    auto begin = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
    auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    }).base();

    if (begin >= end) {
        return {};
    }

    return std::string(begin, end);
}

bool equalsIgnoreCase(const std::string &lhs, const std::string &rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(lhs[i])) != std::tolower(static_cast<unsigned char>(rhs[i]))) {
            return false;
        }
    }

    return true;
}

bool isLanToken(const std::string &value) {
    if (value.empty()) {
        return false;
    }

    std::string trimmed = trimCopy(value);
    return equalsIgnoreCase(trimmed, "LAN") || equalsIgnoreCase(trimmed, "Local Area Network");
}
}

ServerBrowserController::ServerBrowserController(ClientEngine &engine,
                                                 ClientConfig &clientConfig,
                                                 const std::string &configPath,
                                                 const std::string &defaultHost,
                                                 uint16_t defaultPort,
                                                 ServerConnector &connector)
    : engine(engine),
      clientConfig(clientConfig),
      clientConfigPath(configPath),
            connector(connector),
            defaultHost(defaultHost.empty() ? "localhost" : defaultHost),
            defaultPort(defaultPort == 0 ? 1234 : defaultPort) {
    refreshGuiServerListOptions();
    rebuildServerListFetcher();

        engine.gui->showServerBrowser({}, this->defaultHost, this->defaultPort);
    triggerFullRefresh();
    nextAutoScanTime = SteadyClock::now() + autoScanInterval;
}

void ServerBrowserController::triggerFullRefresh() {
    bool lanActive = isLanSelected();
    bool issuedRequest = false;

    if (lanActive) {
        discovery.startScan();
        issuedRequest = true;
    }

    if (serverListFetcher) {
        serverListFetcher->requestRefresh();
        issuedRequest = true;
    }

    if (!issuedRequest) {
        engine.gui->setServerBrowserStatus("No server sources configured. Add a server list or enable Local Area Network.", true);
        engine.gui->setServerBrowserScanning(false);
        return;
    }

    std::string selectionLabel = "selected server list";
    if (!lanActive) {
        if (const auto *source = getSelectedRemoteSource()) {
            selectionLabel = resolveDisplayNameForSource(*source);
        }
    }

    if (lanActive && serverListFetcher) {
        engine.gui->setServerBrowserStatus("Searching local network and fetching the selected server list...", false);
    } else if (lanActive) {
        engine.gui->setServerBrowserStatus("Searching local network for servers...", false);
    } else {
        engine.gui->setServerBrowserStatus("Fetching " + selectionLabel + "...", false);
    }

    engine.gui->setServerBrowserScanning(issuedRequest);
}

void ServerBrowserController::rebuildEntries() {
    const auto &servers = discovery.getServers();
    const bool lanViewActive = isLanSelected();

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
        const std::string addressLabel = serverInfo.host + ":" + std::to_string(serverInfo.port);
        entry.label = lanViewActive ? addressLabel : (serverInfo.name.empty() ? "LAN server" : serverInfo.name);
        entry.host = serverInfo.host;
        entry.port = serverInfo.port;
        entry.description = serverInfo.world.empty() ? "Discovered via broadcast" : serverInfo.world;
        if (lanViewActive && !serverInfo.name.empty()) {
            entry.description = serverInfo.name;
            if (!serverInfo.world.empty()) {
                entry.description += " — " + serverInfo.world;
            }
        }
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

    lastGuiEntries = entries;
    engine.gui->setServerBrowserEntries(lastGuiEntries);
    if (!entries.empty()) {
        engine.gui->setServerBrowserStatus("Select a server to connect.", false);
    }
}

void ServerBrowserController::update() {
    if (auto listSelection = engine.gui->consumeServerBrowserListSelection()) {
        handleServerListSelection(*listSelection);
    }

    if (auto newList = engine.gui->consumeServerBrowserNewListRequest()) {
        handleServerListAddition(*newList);
    }

    auto nowSteady = SteadyClock::now();

    if (engine.gui->consumeServerBrowserRefreshRequest()) {
        triggerFullRefresh();
        nextAutoScanTime = nowSteady + autoScanInterval;
    } else if (lanAutoRefreshEnabled && !discovery.isScanning() && nowSteady >= nextAutoScanTime) {
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
            updateServerListDisplayNamesFromCache();
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

    const ClientServerListSource *selectedRemoteSource = getSelectedRemoteSource();
    std::string remoteLabel = selectedRemoteSource ? resolveDisplayNameForSource(*selectedRemoteSource) : "selected server list";

    if (lanEmpty && remoteEmpty) {
        if (discovery.isScanning() && isLanSelected()) {
            engine.gui->setServerBrowserStatus("Searching local network for servers...", false);
        } else if (remoteFetchingActive && serverListFetcher) {
            engine.gui->setServerBrowserStatus("Fetching " + remoteLabel + "...", false);
        } else if (isLanSelected()) {
            engine.gui->setServerBrowserStatus("No LAN servers found. Start one locally or refresh.", true);
        } else if (serverListFetcher) {
            engine.gui->setServerBrowserStatus(remoteLabel + " returned no servers. Verify the list provider.", true);
        } else {
            engine.gui->setServerBrowserStatus("No server sources configured. Add a server list or enable Local Area Network.", true);
        }
    }
}

void ServerBrowserController::handleDisconnected(const std::string &reason) {
    std::string status = reason.empty()
        ? std::string("Disconnected from server. Select a server to reconnect.")
        : reason;

    engine.gui->showServerBrowser(lastGuiEntries, defaultHost, defaultPort);
    engine.gui->setServerBrowserStatus(status, true);
    triggerFullRefresh();
    nextAutoScanTime = SteadyClock::now() + autoScanInterval;
}

void ServerBrowserController::refreshGuiServerListOptions() {
    std::vector<GUI::ServerListOption> options;

    if (clientConfig.showLanServers) {
        GUI::ServerListOption lanOption;
        lanOption.name = "Local Area Network";
        options.push_back(std::move(lanOption));
    }

    for (const auto &source : clientConfig.serverLists) {
        GUI::ServerListOption option;
        option.name = resolveDisplayNameForSource(source);
        option.url = source.url;
        options.push_back(std::move(option));
    }

    int optionCount = static_cast<int>(options.size());
    if (optionCount == 0) {
        activeServerListIndex = -1;
    } else if (activeServerListIndex < 0 || activeServerListIndex >= optionCount) {
        int desiredIndex = computeDefaultSelectionIndex(optionCount);
        if (desiredIndex < 0 || desiredIndex >= optionCount) {
            desiredIndex = 0;
        }
        activeServerListIndex = desiredIndex;
    }

    engine.gui->setServerBrowserListOptions(options, activeServerListIndex);
}

std::vector<ClientServerListSource> ServerBrowserController::resolveActiveServerLists() const {
    std::vector<ClientServerListSource> result;
    if (const auto *source = getSelectedRemoteSource()) {
        result.push_back(*source);
    }
    return result;
}

void ServerBrowserController::rebuildServerListFetcher() {
    auto sources = resolveActiveServerLists();
    if (sources.empty()) {
        serverListFetcher.reset();
        cachedRemoteServers.clear();
        lastServerListGeneration = 0;
        return;
    }

    serverListFetcher = std::make_unique<ServerListFetcher>(std::move(sources));
    cachedRemoteServers.clear();
    lastServerListGeneration = 0;
    serverListFetcher->requestRefresh();
}

void ServerBrowserController::handleServerListSelection(int selectedIndex) {
    int optionCount = totalListOptionCount();
    if (optionCount == 0) {
        return;
    }

    if (selectedIndex < 0) {
        selectedIndex = 0;
    } else if (selectedIndex >= optionCount) {
        selectedIndex = optionCount - 1;
    }

    if (selectedIndex == activeServerListIndex) {
        return;
    }

    activeServerListIndex = selectedIndex;
    rebuildServerListFetcher();
    rebuildEntries();

    if (isLanSelected()) {
        engine.gui->setServerBrowserListStatus("Local Area Network selected.", false);
    } else {
        engine.gui->setServerBrowserListStatus("Server list updated.", false);
    }

    triggerFullRefresh();
}

void ServerBrowserController::handleServerListAddition(const GUI::ServerListOption &option) {
    std::string trimmedUrl = trimCopy(option.url);

    if (trimmedUrl.empty()) {
        engine.gui->setServerBrowserListStatus("Enter a URL before saving.", true);
        return;
    }

    auto existing = std::find_if(clientConfig.serverLists.begin(), clientConfig.serverLists.end(),
        [&](const ClientServerListSource &source) {
            return source.url == trimmedUrl;
        });
    if (existing != clientConfig.serverLists.end()) {
        engine.gui->setServerBrowserListStatus("A server list with that URL already exists.", true);
        return;
    }

    ClientServerListSource source;
    source.url = trimmedUrl;
    clientConfig.serverLists.push_back(source);

    if (!clientConfig.Save(clientConfigPath)) {
        clientConfig.serverLists.pop_back();
        engine.gui->setServerBrowserListStatus("Failed to write config_client.json. Check permissions.", true);
        return;
    }

    engine.gui->setServerBrowserListStatus("Server list saved.", false);
    engine.gui->clearServerBrowserNewListInputs();

    activeServerListIndex = getLanOffset() + static_cast<int>(clientConfig.serverLists.size()) - 1;
    refreshGuiServerListOptions();
    rebuildServerListFetcher();
    triggerFullRefresh();
}

void ServerBrowserController::updateServerListDisplayNamesFromCache() {
    bool changed = false;
    for (const auto &record : cachedRemoteServers) {
        if (record.sourceUrl.empty() || record.sourceName.empty()) {
            continue;
        }
        auto it = serverListDisplayNames.find(record.sourceUrl);
        if (it == serverListDisplayNames.end() || it->second != record.sourceName) {
            serverListDisplayNames[record.sourceUrl] = record.sourceName;
            changed = true;
        }
    }

    if (changed) {
        refreshGuiServerListOptions();
    }
}

std::string ServerBrowserController::resolveDisplayNameForSource(const ClientServerListSource &source) const {
    auto it = serverListDisplayNames.find(source.url);
    if (it != serverListDisplayNames.end() && !it->second.empty()) {
        return it->second;
    }

    if (!source.name.empty()) {
        return source.name;
    }

    return source.url;
}

int ServerBrowserController::getLanOffset() const {
    return clientConfig.showLanServers ? 1 : 0;
}

int ServerBrowserController::totalListOptionCount() const {
    return getLanOffset() + static_cast<int>(clientConfig.serverLists.size());
}

bool ServerBrowserController::isLanIndex(int index) const {
    return clientConfig.showLanServers && index == 0;
}

bool ServerBrowserController::isLanSelected() const {
    return isLanIndex(activeServerListIndex);
}

const ClientServerListSource* ServerBrowserController::getSelectedRemoteSource() const {
    if (activeServerListIndex < 0) {
        return nullptr;
    }

    int lanOffset = getLanOffset();
    if (activeServerListIndex < lanOffset) {
        return nullptr;
    }

    int remoteIndex = activeServerListIndex - lanOffset;
    if (remoteIndex < 0 || remoteIndex >= static_cast<int>(clientConfig.serverLists.size())) {
        return nullptr;
    }

    return &clientConfig.serverLists[remoteIndex];
}

int ServerBrowserController::computeDefaultSelectionIndex(int optionCount) const {
    if (optionCount == 0) {
        return -1;
    }

    std::string trimmedDefault = trimCopy(clientConfig.defaultServerList);
    if (clientConfig.showLanServers) {
        if (trimmedDefault.empty() || isLanToken(trimmedDefault)) {
            return 0;
        }
    }

    if (!trimmedDefault.empty()) {
        for (std::size_t i = 0; i < clientConfig.serverLists.size(); ++i) {
            if (clientConfig.serverLists[i].url == trimmedDefault) {
                return getLanOffset() + static_cast<int>(i);
            }
        }
    }

    if (clientConfig.showLanServers) {
        return 0;
    }

    return 0;
}
