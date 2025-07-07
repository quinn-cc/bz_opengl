#include <enet.h>
#include <iostream>
#include <cstring>
#include <spdlog/spdlog.h>
#include <math.h>
#include "cxxopts.hpp"
#include "netmsg.hpp"
#include "client.hpp"
#include "client.hpp"
#include "shot.hpp"
#include "renderer.hpp"
#include "networker.hpp"
#include "player.hpp"
#include "input.hpp"

void ParseArgs(int argc, char *argv[]) {
    cxxopts::Options options("BZ", "This is the client.");
    options.add_options()
        ("n,name", "Name to display", cxxopts::value<std::string>()->default_value("default"));
    auto result = options.parse(argc, argv);
    Player::GetInstance().SetName(result["name"].as<std::string>());
}

void Start() {
    Player::GetInstance().Init();
    Renderer::GetInstance().Init();
    Networker::GetInstance().Init();
    Networker::GetInstance().MsgSend_Init();
}

void Update() {
    Networker::GetInstance().Update();
    Input::GetInstance().Update();

    for (Shot *shot : Shot::shots) {
        shot->Update();
    }

    Player::GetInstance().Update();

    Renderer::GetInstance().BeginFrame();
    Renderer::GetInstance().Update();

    for (Client *client : Client::clients) {
        Renderer::GetInstance().Draw(client);
    }

    for (Shot *shot : Shot::shots) {
        Renderer::GetInstance().Draw(shot);
    }
    
    Renderer::GetInstance().EndFrame();
}

void Close() {
    Networker::GetInstance().Close();
    Renderer::GetInstance().Close();
}

int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::debug);
    ParseArgs(argc, argv);
    Start();

    while (!WindowShouldClose()) {
        Update();
    }

    Close();
    return 0;
}