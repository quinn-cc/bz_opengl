#pragma once
#include "engine/types.hpp"
#include "engine/components/server_network.hpp"
#include "engine/components/physics.hpp"

class ServerEngine {
public:
    ServerNetwork *network;
    Physics *physics;

    ServerEngine(uint16_t serverPort);
    ~ServerEngine();

    void earlyUpdate(TimeUtils::duration deltaTime);
    void lateUpdate(TimeUtils::duration deltaTime);
};