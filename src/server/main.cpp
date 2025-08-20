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
#include "runtime.hpp"

void Update() {
    Networker::GetInstance().Update();
    Runtime::GetInstance().Update();

    for (Shot *shot : Shot::shots) {
        shot->Update();
    }
}

int main(int argc, char** argv) {
    spdlog::set_level(spdlog::level::debug);
    Networker::GetInstance().Init();
    Runtime::GetInstance().Init();

    while (true) {
        Update();
    }

    Networker::GetInstance().Close();
    Runtime::GetInstance().Close();
    return 0;
}

