#pragma once
#include "geometry.hpp"
#include "types.hpp"
#include <string>
#include <glm/glm.hpp>

namespace Api {
    Location bz_getLocation(client_id clientId);
    void bz_setLocation(client_id clientId, Location location);
    glm::vec3 bz_getVelocity(client_id clientId);
    void bz_setVelocity(client_id clientId, glm::vec3 velocity);
    bool bz_isAlive(client_id clientId);
    void bz_spawn(client_id clientId, Location location);
    void bz_kill(client_id clientId);
    std::string bz_getName(client_id clientId);
    void bz_setName(client_id clientId, const std::string &name);
    client_id bz_getId();
    void bz_fireShot(client_id clientId, glm::vec3 position, glm::vec3 velocity);
    void bz_removeShot(shot_id shotId);
}