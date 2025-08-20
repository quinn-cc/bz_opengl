#include "client.hpp"
#include <spdlog/spdlog.h>

std::vector<Client *> Client::clients;
std::vector<std::function<void(Client*)>> Client::callbacks_add;
std::vector<std::function<void(Client*)>> Client::callbacks_remove;

Client *Client::GetClient(int clientId) {
    for (Client *client : clients) {
        if (client->IsClient(clientId)) {
            return client;
        }
    }

    return nullptr;
}

void Client::AddCallback_AddClient(std::function<void(Client *)> func) {
    Client::callbacks_add.push_back(std::move(func));
}

void Client::AddCallback_RemoveClient(std::function<void(Client *)> func) {
    Client::callbacks_remove.push_back(std::move(func));
}

Client::Client(int clientId, std::string name) {
    this->clientId = clientId;
    this->name = name;

    memset(&this->location, '\0', sizeof(this->location));
    locationTime = std::chrono::system_clock::now();
    lastLocationTime = std::chrono::system_clock::now();

    clients.push_back(this);

    for (auto &cb : Client::callbacks_add) {
        cb(this);
    }
}

Client::~Client() {
    clients.erase(std::remove(clients.begin(), clients.end(), this), clients.end());

    for (auto &cb : Client::callbacks_remove) {
        cb(this);
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
    // auto now = std::chrono::system_clock::now();
    // auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - locationTime).count();
    // auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(locationTime - lastLocationTime).count();

    // float alpha = 1;
    // if (totalMs != 0) 
    //     alpha = static_cast<float>(ms) / static_cast<float>(totalMs);
    // alpha = glm::clamp(alpha, 0.f, 1.f);

    // Location loc;
    // loc.position = glm::mix(lastLocation.position, location.position, alpha);
    // loc.rotation = glm::normalize(glm::slerp(lastLocation.rotation, location.rotation, alpha));
    // return loc;
    return location;
}

void Client::SetLocation(Location location) {
    this->lastLocation = this->location;
    lastLocationTime = locationTime;
    this->location = location;
    locationTime = std::chrono::system_clock::now();
}

