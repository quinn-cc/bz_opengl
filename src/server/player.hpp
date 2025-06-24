#pragma once
#include <string>
#include <enet.h>
#include "raylib.h"

class Player {
public:
    ENetHost* server;
    ENetPeer *peer;
    int clientId;

    std::string name;
    Vector3 position;
    Quaternion rotation;

    void sendPacket(void *packet, int len);
    bool isPeer(ENetPeer *peer);
    std::string toString();

    Player();
};