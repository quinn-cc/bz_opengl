#pragma once
#include <glm/glm.hpp>

using client_id = unsigned int;
using shot_id = unsigned int;

typedef struct InputMap {
    bool fire;
    bool spawn;
    bool jump;
    glm::vec2 movement;
} InputMap;