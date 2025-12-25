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
    std::array<char, 256> inputBuf{};
    std::string submittedInputBuffer;
    bool hasNewInputBuffer = false;
    bool chatFocus = false;

    void drawTexture(unsigned int textureId);
    void drawConsolePanel();

    GUI(GLFWwindow *window);
    ~GUI();

public:
    void setScoreboardPlayerNames(const std::vector<std::string> &names);
    void addConsoleLine(const std::string &playerName, const std::string &line);
    std::string getChatInputBuffer() const;
    bool hasChatInputBuffer() const;
    void setChatInputFocus(bool focus);
};