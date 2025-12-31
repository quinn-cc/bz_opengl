#include "shot.hpp"
#include <glm/glm.hpp>
#include "renderer.hpp"
#include <functional>
#include <vector>

shot_id Shot::GenerateLocalShotId() {
    static shot_id currentId = 1;
    return currentId++;
}

void Shot::Update(float deltaTime) {
    position = position + (velocity * deltaTime);
}