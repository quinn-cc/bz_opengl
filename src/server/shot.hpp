#pragma once
#include <glm/glm.hpp>
#include <vector>

class Client;

class Shot {
private:
    glm::vec3 position;
    glm::vec3 velocity;
    Client *owner;

public:
    static std::vector<Shot *> shots;

    Shot(Client *owner, glm::vec3 position, glm::vec3 velocity);
    void Update();
};