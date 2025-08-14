#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

typedef struct Location {
    glm::vec3 position;
    glm::quat rotation;
} Location;