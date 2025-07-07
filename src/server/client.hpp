#pragma once
#include <string>
#include <vector>
#include "raylib.h"
#include "geometry.hpp"

class Client {
private:
    int clientId;

    std::string name;
    Location location;
    
public:
    static std::vector<Client *> clients;
    static Client *GetClient(int clientId);
    static Client *RemoveClient(int clientId);

    void SendPacket(void *packet, int len);
    bool IsPeer(int clientId);
    std::string GetName();
    int GetId();
    void SetLocation(Location location);
    Location GetLocation();
    std::string ToString();

    Client(std::string name);
};

int GenerateClientId();