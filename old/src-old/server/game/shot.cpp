#include "shot.hpp"
#include "runtime.hpp"
#include <glm/glm.hpp>
#include "networker.hpp"
#include <spdlog/spdlog.h>

Shot::Shot(shot_id localId, Client *owner, glm::vec3 position, glm::vec3 velocity) {
    this->position = position;
    this->velocity = velocity;
    this->owner = owner;
    this->startTime = TimeUtils::GetCurrentTime();
    this->localId = localId;
    this->globalId = GenerateShotId();
}

void Shot::Update(TimeUtils::duration deltaTime) {
    position = position + (velocity * deltaTime);
}

bool Shot::IsExpired() const {
    return TimeUtils::GetElapsedTime(startTime, TimeUtils::GetCurrentTime()) > SHOT_LIFETIME;
}