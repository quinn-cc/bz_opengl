#pragma once
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>      // your platform backend
#include <imgui_impl_opengl3.h>   // your renderer backend
#include <vector>
#include <string>
#include <array>
#include <optional>
#include <cstdint>

class GUI {
    friend class ClientEngine;

public:
    struct ServerBrowserEntry {
        std::string label;
        std::string host;
        uint16_t port;
        std::string description;
        std::string displayHost;
    };

    struct ServerBrowserSelection {
        std::string host;
        uint16_t port;
        bool fromPreset;
    };

private:
    GLFWwindow *window;
    ImFont* bigFont;

    void update();
    std::vector<std::string> scoreboardPlayerNames;
    std::vector<std::string> consoleLines;

    std::array<char, 256> chatInputBuffer{};
    std::string submittedInputBuffer;
    bool chatFocus = false;

    void drawTexture(unsigned int textureId);
    void drawConsolePanel();
    void drawDeathScreen();
    void drawServerBrowser();
    bool drawDeathScreenFlag = false;

    GUI(GLFWwindow *window);
    ~GUI();

public:
    void setScoreboardPlayerNames(const std::vector<std::string> &names);
    void addConsoleLine(const std::string &playerName, const std::string &line);
    std::string getChatInputBuffer() const;
    void clearChatInputBuffer();
    void focusChatInput();
    bool getChatInputFocus() const;
    void showServerBrowser(const std::vector<ServerBrowserEntry> &entries,
        const std::string &defaultHost,
        uint16_t defaultPort);
    void setServerBrowserEntries(const std::vector<ServerBrowserEntry> &entries);
    void hideServerBrowser();
    bool isServerBrowserVisible() const;
    void setServerBrowserStatus(const std::string &statusText, bool isErrorMessage);
    std::optional<ServerBrowserSelection> consumeServerBrowserSelection();
    bool consumeServerBrowserRefreshRequest();
    void setServerBrowserScanning(bool scanning);
    void displayDeathScreen(bool show);

private:
    bool showServerBrowserFlag = false;
    std::vector<ServerBrowserEntry> serverBrowserEntries;
    int serverBrowserSelectedIndex = -1;
    std::array<char, 256> serverBrowserHostBuffer{};
    std::array<char, 6> serverBrowserPortBuffer{};
    std::string serverBrowserStatusText;
    bool serverBrowserStatusIsError = false;
    std::optional<ServerBrowserSelection> pendingServerBrowserSelection;
    bool serverBrowserRefreshRequested = false;
    bool serverBrowserScanning = false;

    void resetServerBrowserBuffers(const std::string &defaultHost, uint16_t defaultPort);
};