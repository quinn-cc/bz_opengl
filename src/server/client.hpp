#pragma once
#include <string>
#include <vector>
#include "geometry.hpp"
#include "types.hpp"

class Client {
private:
    client_id clientId;
    std::string name;
    Location location;
    
public:
    static std::vector<Client *> clients;
    static Client *GetClient(client_id clientId);
    static Client *RemoveClient(client_id clientId);
    static client_id GenerateClientId();

    void SendPacket(void *packet, int len);
    bool IsPeer(client_id clientId);
    std::string GetName();
    client_id GetId();
    void SetLocation(Location location);
    Location GetLocation();
    std::string ToString();

    Client(std::string name);
    ~Client();
};