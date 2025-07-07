#include "client.hpp"
#include <spdlog/spdlog.h>

std::vector<Client *> Client::clients;

Client *Client::GetClient(int clientId) {
    for (Client *client : clients) {
        if (client->IsClient(clientId)) {
            return client;
        }
    }

    return nullptr;
}

Client::Client(int clientId, std::string name) {
    this->clientId = clientId;
    this->name = name;

    memset(&this->location, '\0', sizeof(this->location));
    locationTime = std::chrono::system_clock::now();
    lastLocationTime = std::chrono::system_clock::now();

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

std::string Client::ToString() {
    std::string out = "";
    out += "Client{name=";
    out += name;
    out += ",clientId=";
    out += std::to_string(clientId);
    out += "}";
    return out;
}

bool Client::IsClient(int clientId) const {
    return this->clientId == clientId;
}

bool Client::IsClient(std::string name) const {
    return this->name == name;
}

Location Client::GetLocation() const {
    return location;
}

std::string Client::GetName() const {
    return name;
}

Location Client::GetInterpolatedLocation() const {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - locationTime).count();
    auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(locationTime - lastLocationTime).count();

    float alpha = 1;

    if (ms < totalMs)
        alpha = static_cast<float>(ms) / static_cast<float>(totalMs);

    Location loc;
    loc.position = glm::mix(lastLocation.position, location.position, alpha);
    loc.rotation = glm::normalize(glm::slerp(lastLocation.rotation, location.rotation, alpha));
    return loc;
}

void Client::SetLocation(Location location) {
    this->lastLocation = this->location;
    lastLocationTime = locationTime;
    this->location = location;
    locationTime = std::chrono::system_clock::now();
}

