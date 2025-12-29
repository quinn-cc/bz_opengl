#include "world.hpp"
#include "game.hpp"
#include <string>

World::World(Game &game, std::string worldDir) : game(game) {
    settings = DEFAULT_WORLD_SETTINGS;

    physicsId = game.engine.physics->create(worldDir + "/world.glb", 0.0f);
}

World::~World() {
    game.engine.physics->destroy(physicsId);
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
    // Make x random between -10 and 10 and make z random between -10 and 10
    // and make z random between 10 and 15, and y rotation random between 0 and 2pi
    float x = static_cast<float>(rand() % 2001 - 1000) / 100.0f;
    float z = static_cast<float>(rand() % 2001 - 1000) / 100.0f;
    float y = static_cast<float>(rand() % 501 + 1000) / 100.0f;
    float rotY = static_cast<float>(rand() % 6283) / 1000.0f;
    return Location{
        .position = glm::vec3(x, y, z),
        .rotation = glm::angleAxis(rotY, glm::vec3(0, 1, 0))
    };
}