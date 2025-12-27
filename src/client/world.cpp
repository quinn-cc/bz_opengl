#include "world.hpp"
#include "game.hpp"
#include "spdlog/spdlog.h"

World::World(Game &game) : game(game) {
    settings = DEFAULT_WORLD_SETTINGS;

    renderId = game.engine.render->create("data/world2.glb");
    physicsId = game.engine.physics->create("data/world2.glb", 0.0f);

    game.engine.physics->setGravity(getSetting("gravity"));
}

World::~World() {
    game.engine.render->destroy(renderId);
    game.engine.physics->destroy(physicsId);
}

float World::getSetting(const std::string &key) const {
    auto it = settings.find(key);
    if (it != settings.end()) {
        return it->second;
    } else {
        throw std::runtime_error("Key not found in World settings: " + key);
    }
}

void World::setSetting(const std::string &key, float value) {
    spdlog::info("World setting changed: {} = {}", key, value);

    settings[key] = value;

    if (key == "gravity") {
        game.engine.physics->setGravity(value);
    }
}

void World::update() {
    // Listen for incoming world setting changes
    if (auto *settingChangeMsg = game.engine.network->peekMessage<ServerMsg_WorldSettingChange>()) {
        std::string key = std::string(settingChangeMsg->key);
        float value = settingChangeMsg->value;
        setSetting(key, value);
        game.engine.network->popMessage(settingChangeMsg);
    }
}