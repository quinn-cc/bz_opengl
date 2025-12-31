#include "world.hpp"
#include "game.hpp"
#include "spdlog/spdlog.h"

World::World(Game &game) : game(game) {
    renderId = game.engine.render->create("data/world2.glb");
    physicsId = game.engine.physics->create("data/world2.glb", 0.0f);
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

bool World::isInitialized() const {
    return initialized;
}

void World::update() {
    if (auto *initMsg = game.engine.network->peekMessage<ServerMsg_Init>()) {
        spdlog::trace("World::update: Received init message from server");
        // Load settings from init message
        settings = initMsg->settings;

        // Update all settings
        for (const auto& [key, value] : settings) {
            setSetting(key, value);
        }

        spdlog::info("World::update: World initialized from server");
        initialized = true;
        return;
    }

    // Listen for incoming world setting changes
    if (auto *settingChangeMsg = game.engine.network->peekMessage<ServerMsg_WorldSettingChange>()) {
        std::string key = std::string(settingChangeMsg->key);
        float value = settingChangeMsg->value;
        setSetting(key, value);
    }
}