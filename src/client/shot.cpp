#include "shot.hpp"
#include <glm/glm.hpp>
#include "renderer.hpp"
#include <functional>
#include <vector>

std::vector<Shot *> Shot::shots;
std::vector<std::function<void(Shot*)>> Shot::callbacks_add;
std::vector<std::function<void(Shot*)>> Shot::callbacks_remove;

Shot *Shot::GetShotByGlobalId(shot_id globalId) {
    for (auto *shot : shots) {
        if (shot->ownerId != 0 && shot->GetId() == globalId) {
            return shot;
        }
    }
    return nullptr;
}

Shot *Shot::GetShotByLocalId(shot_id localId) {
    for (auto *shot : shots) {
        if (shot->ownerId == 0 && shot->GetId() == localId) {
            return shot;
        }
    }
    return nullptr;
}

shot_id Shot::GenerateLocalShotId() {
    static shot_id currentId = 0;
    return currentId++;
}

void Shot::AddCallback_AddShot(std::function<void(Shot *)> func) {
    Shot::callbacks_add.push_back(std::move(func));
}

void Shot::AddCallback_RemoveShot(std::function<void(Shot *)> func) {
    Shot::callbacks_remove.push_back(std::move(func));
}

Shot::Shot(shot_id globalId, glm::vec3 position, glm::vec3 velocity) {
    this->position = position;
    this->velocity = velocity;
    this->id = globalId;
    this->ownerId = 1;
    shots.push_back(this);

    for (auto &cb : Shot::callbacks_add) {
        cb(this);
    }
}

Shot::~Shot() {
    shots.erase(std::remove(shots.begin(), shots.end(), this), shots.end());

    for (auto &cb : Shot::callbacks_remove) {
        cb(this);
    }    
}

void Shot::Update() {
    position = position + (velocity + Renderer::GetInstance().GetDeltaTime());    
}

glm::vec3 Shot::GetPosition() {
    return position;
}

glm::vec3 Shot::GetVelocity() {
    return velocity;
}

shot_id Shot::GetId() const {
    return id;
}
