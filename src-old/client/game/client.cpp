#include "client.hpp"
#include <spdlog/spdlog.h>

Client::Client(client_id clientId, std::string name) {
    this->clientId = clientId;
    this->name = name;

    memset(&this->location, '\0', sizeof(this->location));
    locationTime = TimeUtils::GetCurrentTime();
    lastLocationTime = TimeUtils::GetCurrentTime();
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