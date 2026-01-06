#include <GLFW/glfw3.h>
#include <memory>
#include "spdlog/spdlog.h"
#include "engine/client_engine.hpp"
#include "game.hpp"
#include "cxxopts.hpp"
#include "client/config_client.hpp"
#include "client/server_browser_controller.hpp"

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
    const bool autoConnectRequested = result.count("addr") > 0;

    constexpr const char *kClientConfigPath = "../data/config_client.json";
    ClientConfig clientConfig = ClientConfig::Load(kClientConfigPath);

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

    ServerBrowserController serverBrowser(engine, clientConfig, connectAddr, connectPort);
    std::unique_ptr<Game> game;

    auto connectToServer = [&](const std::string &targetHost, uint16_t targetPort) {
        std::string status = "Connecting to " + targetHost + ":" + std::to_string(targetPort) + "...";
        engine.gui->setServerBrowserStatus(status, false);
        spdlog::info("Attempting to connect to {}:{}", targetHost, targetPort);

        if (engine.network->connect(targetHost, targetPort, 50)) {
            spdlog::info("Connected to server at {}:{}", targetHost, targetPort);
            game = std::make_unique<Game>(engine, playerName, worldDir);
            spdlog::trace("Game initialized successfully");
            engine.gui->hideServerBrowser();
            return true;
        }

        spdlog::error("Failed to connect to server at {}:{}", targetHost, targetPort);
        std::string errorMsg = "Unable to reach " + targetHost + ":" + std::to_string(targetPort) + ".";
        engine.gui->setServerBrowserStatus(errorMsg, true);
        return false;
    };

    if (autoConnectRequested) {
        connectToServer(connectAddr, connectPort);
    }

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
            serverBrowser.update();
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
                connectToServer(selection->host, selection->port);
            }
        }

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}