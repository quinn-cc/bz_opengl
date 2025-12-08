#pragma once
#include <string>

class World {
private:
    float playerSpeed;
    float playerTurnSpeed;

public:
    World() = default;
    ~World() = default;

    void load(std::string worldPath);

    float getPlayerSpeed() const;
    float getPlayerTurnSpeed() const;
};