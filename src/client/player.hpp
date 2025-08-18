#pragma once
#include <glm/glm.hpp>
#include <string>
#include "geometry.hpp"

class Player {
private:
    Location location;
    Location lastLocation;
    std::string name;
    float moveSpeed;
    float turnSpeed;
    float getYaw();
    float getLastYaw();

public:
    static Player &GetInstance();
    
    void Init();
    void Update();
    void SetName(std::string name);
    std::string GetName();
    glm::vec3 GetForwardVector();
    Location GetLocation();
    void SetLocation(Location location);
};