#pragma once
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <array>

class GUI {
    friend class ClientEngine;

private:
    GLFWwindow *window;
    void update();
    std::vector<std::string> scoreboardPlayerNames;
    std::vector<std::string> consoleLines;

    std::array<char, 256> chatInputBuffer{};
    std::string submittedInputBuffer;
    bool chatFocus = false;

    void drawTexture(unsigned int textureId);
    void drawConsolePanel();

    GUI(GLFWwindow *window);
    ~GUI();

public:
    void setScoreboardPlayerNames(const std::vector<std::string> &names);
    void addConsoleLine(const std::string &playerName, const std::string &line);
    std::string getChatInputBuffer() const;
    void clearChatInputBuffer();
    void focusChatInput();
    bool getChatInputFocus() const;
};