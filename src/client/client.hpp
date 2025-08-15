#pragma once
#include "geometry.hpp"
#include <threepp/threepp.hpp>
#include <string>
#include <ctime>
#include <chrono>
#include <vector>

class Client {
private:
    int clientId;
    std::string name;
    Location location;
    Location lastLocation;
    std::chrono::time_point<std::chrono::system_clock> locationTime;
    std::chrono::time_point<std::chrono::system_clock> lastLocationTime;

    static std::vector<std::function<void(Client*)>> callbacks_add;
    static std::vector<std::function<void(Client*)>> callbacks_remove;

public:
    static std::vector<Client *> clients;
    static Client *GetClient(int clientId);
    static void AddCallback_AddClient(std::function<void(Client*)> func);
    static void AddCallback_RemoveClient(std::function<void(Client*)> func);

    Client(int clientId, std::string name);
    ~Client();
    std::string ToString();

    bool IsClient(int clientId) const;
    bool IsClient(std::string name) const;

    std::string GetName() const;

    Location GetLocation() const;
    Location GetInterpolatedLocation() const;
    void SetLocation(Location location);


};
