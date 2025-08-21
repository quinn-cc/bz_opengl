#include "client.hpp"
#include <string.h>
#include <vector>
#include <algorithm>
#include <spdlog/spdlog.h>

std::vector<Client *> Client::clients;

client_id Client::GenerateClientId() {
    static client_id id = 0;
    return id++;
}

Client *Client::GetClient(client_id clientId) {
    for (Client *client : clients) {
        if (client->IsPeer(clientId)) {
            return client;
        }
    }

    return nullptr;
}


bool Client::IsPeer(client_id clientId) {
    return this->clientId == clientId;
}

std::string Client::GetName() {
    return name;
}

client_id Client::GetId() {
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

