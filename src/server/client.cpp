#include "client.hpp"
#include <string.h>
#include <vector>
#include <algorithm>
#include <spdlog/spdlog.h>

std::vector<Client *> Client::clients;

Client *Client::GetClient(int clientId) {
    for (Client *client : clients) {
        if (client->IsPeer(clientId)) {
            return client;
        }
    }

    return nullptr;
}


bool Client::IsPeer(int clientId) {
    return this->clientId == clientId;
}

std::string Client::GetName() {
    return name;
}

int Client::GetId() {
    return clientId;
}

void Client::SetLocation(Location location) {
    this->location = location;
}

Location Client::GetLocation() {
    return location;
}

std::string Client::ToString() {
    std::string out = "";
    out += "Client{name=";
    out += name;
    out += ",";
    out += "clientId=";
    out += std::to_string(clientId);
    out += "}";
    return out;
}

Client::Client(std::string name) {
    this->name = name;
    this->clientId = GenerateClientId();
    clients.push_back(this);
}

Client::~Client() {
    for (Client *c : clients) {
        if (c == this) {
            clients.erase(
                std::remove(clients.begin(), clients.end(), c),
                clients.end()
            );
        }
    }
}

int GenerateClientId() {
    return rand();
}