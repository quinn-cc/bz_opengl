#include "shot.hpp"
#include "runtime.hpp"
#include <glm/glm.hpp>
#include "networker.hpp"
#include <spdlog/spdlog.h>

#define BULLET_SIZE 1
#define SHOT_LIFETIME 1.0f

std::vector<Shot *> Shot::shots;

shot_id Shot::GenerateShotId() {
    static shot_id id = 0;
    return id++;
}

Shot::Shot(shot_id localId, Client *owner, glm::vec3 position, glm::vec3 velocity) {
    this->position = position;
    this->velocity = velocity;
    this->owner = owner;
    this->startTime = TimeUtils::GetCurrentTime();
    this->localId = localId;
    this->globalId = GenerateShotId();
    shots.push_back(this);
}

Shot::~Shot() {
    spdlog::debug("Removing shot: localId={}, globalId={}", localId, globalId);
    shots.erase(std::remove(shots.begin(), shots.end(), this), shots.end());
}

void Shot::Update() {
    position = position + (velocity * Runtime::GetInstance().GetDeltaTime());

    if (TimeUtils::GetElapsedTime(startTime, TimeUtils::GetCurrentTime()) > SHOT_LIFETIME) {
        Networker::GetInstance().MsgSend_RemoveShot(this);
        delete this;
    }
}

shot_id Shot::GetLocalId() const {
    return localId;
}

shot_id Shot::GetGlobalId() const {
    return globalId;
}

Client *Shot::GetOwner() const {
    return owner;
}