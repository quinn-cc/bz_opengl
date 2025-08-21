#include <enet.h>
#include <iostream>
#include <cstring>
#include <spdlog/spdlog.h>
#include <math.h>
#include <csignal>
#include <cstdlib>
#include "cxxopts.hpp"
#include "netmsg.hpp"
#include "client.hpp"
#include "shot.hpp"
#include "renderer.hpp"
#include "networker.hpp"
#include "player.hpp"
#include "input.hpp"
#include "physics.hpp"
#include "gui.hpp"

bool exitSignalRecieved = false;

void ParseArgs(int argc, char *argv[]) {
    cxxopts::Options options("BZ", "This is the client.");
    options.add_options()
        ("a,addr", "Address to connect to", cxxopts::value<std::string>()->default_value("localhost"));
    options.add_options()
        ("n,name", "Name to display", cxxopts::value<std::string>()->default_value("default"));
    auto result = options.parse(argc, argv);
    Player::GetInstance().SetName(result["name"].as<std::string>());
    Networker::GetInstance().serverAddress = result["addr"].as<std::string>();
}

void Start() {
    Player::GetInstance().Init();
    Renderer::GetInstance().Init();
    GUI::GetInstance().Init();
    Input::GetInstance().Init();
    Physics::GetInstance().Init();
    Networker::GetInstance().Init();
    Networker::GetInstance().MsgSend_Init();
}

void Update() {
    Networker::GetInstance().Update();
    Input::GetInstance().Update();
    Physics::GetInstance().Update();

    for (Shot *shot : Shot::shots) {
        shot->Update();
    }

    Player::GetInstance().Update();
    Renderer::GetInstance().Update();
    GUI::GetInstance().Update();
    
    Renderer::GetInstance().EndFrame();
}

void Close() {
    spdlog::debug("Closing window");
    Networker::GetInstance().Close();
    Renderer::GetInstance().Close();
    Physics::GetInstance().Close();
    GUI::GetInstance().Close();
}

void SignalHandlerClose(int signum) {
    exitSignalRecieved = true;
}

int main(int argc, char *argv[]) {
    std::signal(SIGINT, SignalHandlerClose);
    spdlog::set_level(spdlog::level::debug);
    ParseArgs(argc, argv);
    Start();

    while (!exitSignalRecieved && !Renderer::GetInstance().ShouldClose()) {
        Update();
    }

    Close();
    return 0;
}