#include "main.hpp"
#include <enet.h>
#include <iostream>
#include <arpa/inet.h>
#include <vector>
#include "netmsg.hpp"
#include "shot.hpp"
#include <spdlog/spdlog.h>
#include <chrono>
#include "networker.hpp"

void Update() {
    for (Shot *shot : Shot::shots) {
        shot->Update();
    }
}

int main(int argc, char** argv) {
    spdlog::set_level(spdlog::level::debug);
    Networker::GetInstance().Init();
    
    while (true) {
        Networker::GetInstance().Update();
        Update();
    }

    Networker::GetInstance().Close();
    return 0;
}

