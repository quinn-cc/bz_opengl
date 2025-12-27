#include "world.hpp"
#include "game.hpp"
#include <string>

World::World(Game &game) : game(game) {
    settings = DEFAULT_WORLD_SETTINGS;
}

void World::setSetting(std::string key, float value) {
    settings[key] = value;

    // Broadcast the setting change to all clients
    ServerMsg_WorldSettingChange msg;
    strcpy(msg.key, key.c_str());
    msg.value = value;
    game.engine.network->sendAll<ServerMsg_WorldSettingChange>(msg);
}

float World::getSetting(std::string key) const {
    auto it = settings.find(key);
    if (it != settings.end()) {
        return it->second;
    } else {
        throw std::runtime_error("Key not found in World settings: " + key);
    }
}

Location World::getSpawnLocation() const {
    Location spawnLocation;
    spawnLocation.position = glm::vec3(0.0f, 10.0f, 0.0f);
    spawnLocation.rotation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));
    return spawnLocation;
}