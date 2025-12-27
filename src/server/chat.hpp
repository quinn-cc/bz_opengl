#pragma once
#include <vector>
#include <string>

class Game;

class Chat {
private:
    Game &game;
    std::vector<std::string> messages;

public:
    Chat(Game &game);
    ~Chat();

    void update();
};