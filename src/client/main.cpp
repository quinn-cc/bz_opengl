#include <GLFW/glfw3.h>
#include <memory>
#include <vector>
#include <chrono>
#include "spdlog/spdlog.h"
#include "engine/client_engine.hpp"
#include "game.hpp"
#include "cxxopts.hpp"
#include "client/server_discovery.hpp"

TimeUtils::time lastFrameTime;

#define MIN_DELTA_TIME (1.0f / 120.0f)

int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::trace);

    if (!glfwInit()) {
        spdlog::error("GLFW failed to initialize");
        exit(1);
    }

    cxxopts::Options options("BZ", "This is the client.");
    options.add_options()
        ("n,name", "Player name", cxxopts::value<std::string>()->default_value("Player"));
    options.add_options()
        ("a,addr", "Connection address", cxxopts::value<std::string>()->default_value("localhost"));
    options.add_options()
        ("p,port", "Connection port", cxxopts::value<uint16_t>()->default_value("1234"));
    options.add_options()
        ("w,world", "World directory", cxxopts::value<std::string>()->default_value("../client-test/"));
    
    auto result = options.parse(argc, argv);
    std::string playerName = result["name"].as<std::string>();
    std::string connectAddr = result["addr"].as<std::string>();
    uint16_t connectPort = result["port"].as<uint16_t>();
    std::string worldDir = result["world"].as<std::string>();

    spdlog::trace("GLFW initialized successfully");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    

    GLFWwindow *window = glfwCreateWindow(800, 600, "BZFlag v3", nullptr, nullptr);
    //glfwSetWindowUserPointer(window, userPointer);
    if (!window) {
        spdlog::error("GLFW window failed to create");
        glfwTerminate();
        exit(1);
    }


    spdlog::trace("GLFW window created successfully");
    glfwMakeContextCurrent(window);
    spdlog::trace("GLFW context made current");

    glEnable(GL_MULTISAMPLE);
    glfwSwapInterval(1); // Enable vsync


    spdlog::info("GLFW_SAMPLES attrib = {}", glfwGetWindowAttrib(window, GLFW_SAMPLES));

    spdlog::info("GL_VENDOR   = {}", (const char*)glGetString(GL_VENDOR));
    spdlog::info("GL_RENDERER = {}", (const char*)glGetString(GL_RENDERER));
    spdlog::info("GL_VERSION  = {}", (const char*)glGetString(GL_VERSION));

    int sb = 0, s = 0;
    glGetIntegerv(GL_SAMPLE_BUFFERS, &sb);
    glGetIntegerv(GL_SAMPLES, &s);
    spdlog::info("GL_SAMPLE_BUFFERS={}, GL_SAMPLES={}", sb, s);

    ClientEngine engine(window);
    spdlog::trace("ClientEngine initialized successfully");

    std::unique_ptr<Game> game;
    ServerDiscovery discovery;
    std::size_t lastDiscoveryVersion = 0;
    using SteadyClock = std::chrono::steady_clock;
    const auto autoScanInterval = std::chrono::seconds(5);
    auto nextAutoScanTime = SteadyClock::now() + autoScanInterval;

    engine.gui->showServerBrowser({}, connectAddr, connectPort);
    engine.gui->setServerBrowserStatus("Searching local network for servers...", false);
    discovery.startScan();
    engine.gui->setServerBrowserScanning(true);

    lastFrameTime = TimeUtils::GetCurrentTime();

    spdlog::trace("Starting main loop");

    while (!glfwWindowShouldClose(window)) {
        TimeUtils::time currTime = TimeUtils::GetCurrentTime();  
        TimeUtils::duration deltaTime = TimeUtils::GetElapsedTime(lastFrameTime, currTime);

        if (deltaTime < MIN_DELTA_TIME) {
            TimeUtils::sleep(MIN_DELTA_TIME - deltaTime);
            continue;
        }

        lastFrameTime = currTime;

        engine.earlyUpdate(deltaTime);

        if (!game) {
            auto nowSteady = SteadyClock::now();

            if (engine.gui->consumeServerBrowserRefreshRequest()) {
                discovery.startScan();
                engine.gui->setServerBrowserScanning(true);
                if (discovery.getServers().empty()) {
                    engine.gui->setServerBrowserStatus("Searching local network for servers...", false);
                }
                nextAutoScanTime = nowSteady + autoScanInterval;
            } else if (!discovery.isScanning() && nowSteady >= nextAutoScanTime) {
                discovery.startScan();
                engine.gui->setServerBrowserScanning(true);
                if (discovery.getServers().empty()) {
                    engine.gui->setServerBrowserStatus("Searching local network for servers...", false);
                }
                nextAutoScanTime = nowSteady + autoScanInterval;
            }

            discovery.update();
            engine.gui->setServerBrowserScanning(discovery.isScanning());

            const auto &servers = discovery.getServers();
            auto discoveryVersion = discovery.getGeneration();
            if (discoveryVersion != lastDiscoveryVersion) {
                lastDiscoveryVersion = discoveryVersion;
                std::vector<GUI::ServerBrowserEntry> entries;
                entries.reserve(servers.size());
                for (const auto &serverInfo : servers) {
                    GUI::ServerBrowserEntry entry;
                    entry.label = serverInfo.name.empty() ? "LAN server" : serverInfo.name;
                    entry.host = serverInfo.host;
                    entry.port = serverInfo.port;
                    entry.description = serverInfo.world.empty() ? "Discovered via broadcast" : serverInfo.world;
                    entry.displayHost = serverInfo.displayHost.empty() ? serverInfo.host : serverInfo.displayHost;
                    entries.push_back(entry);
                }
                engine.gui->setServerBrowserEntries(entries);
                if (!entries.empty()) {
                    engine.gui->setServerBrowserStatus("Select a server to connect.", false);
                }
            }

            if (!discovery.isScanning() && servers.empty()) {
                engine.gui->setServerBrowserStatus("No LAN servers found. Start one or enter a custom host/port.", true);
            }
        }

        if (game) {
            game->earlyUpdate(deltaTime);
        }

        engine.step(deltaTime);

        if (game) {
            game->lateUpdate(deltaTime);
        }

        engine.lateUpdate(deltaTime);

        if (!game) {
            if (auto selection = engine.gui->consumeServerBrowserSelection()) {
                const std::string targetHost = selection->host;
                const uint16_t targetPort = selection->port;
                std::string status = "Connecting to " + targetHost + ":" + std::to_string(targetPort) + "...";
                engine.gui->setServerBrowserStatus(status, false);
                spdlog::info("Attempting to connect to {}:{}", targetHost, targetPort);

                if (engine.network->connect(targetHost, targetPort, 50)) {
                    spdlog::info("Connected to server at {}:{}", targetHost, targetPort);
                    game = std::make_unique<Game>(engine, playerName);
                    spdlog::trace("Game initialized successfully");
                    engine.gui->hideServerBrowser();
                } else {
                    spdlog::error("Failed to connect to server at {}:{}", targetHost, targetPort);
                    std::string errorMsg = "Unable to reach " + targetHost + ":" + std::to_string(targetPort) + ".";
                    engine.gui->setServerBrowserStatus(errorMsg, true);
                }
            }
        }

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}