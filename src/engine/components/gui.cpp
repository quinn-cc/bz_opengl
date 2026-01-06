#include "engine/components/gui.hpp"
#include <GLFW/glfw3.h>
#include "spdlog/spdlog.h"
#include <cstdio>
#include <optional>

GUI::GUI(GLFWwindow *window) {
    // Initialize ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    // Set ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Load fonts
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();

    bigFont = io.Fonts->AddFontFromFileTTF(
        "data/fonts/share_tech_mono_regular.ttf",
        100.0f   // font size in pixels
    );

    io.Fonts->Build();
}

GUI::~GUI() {
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GUI::update() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (showServerBrowserFlag) {
        drawServerBrowser();
    } else {
        // Render HUD (scoreboard, console, crosshair)
        ImGui::SetNextWindowPos(ImVec2(20, 20));
        ImGui::SetNextWindowSize(ImVec2(500, 200));
        ImGui::SetNextWindowBgAlpha(0.0f);

        ImGui::Begin("TopLeftText", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings
        );

        std::string players = "";
        for (const auto& name : scoreboardPlayerNames) {
            players += name + "\n";
        }

        ImGui::Text("%s", players.c_str());
        ImGui::End();

        drawConsolePanel();

        if (drawDeathScreenFlag) {
            drawDeathScreen();
        }

        ImGuiIO& io = ImGui::GetIO();
        const float boxSize = 50.0f;
        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

        ImVec2 p0(
            center.x - boxSize * 0.5f,
            center.y - boxSize * 0.5f
        );

        ImVec2 p1(
            center.x + boxSize * 0.5f,
            center.y + boxSize * 0.5f
        );

        ImGui::GetForegroundDrawList()->AddRect(
            p0,
            p1,
            IM_COL32(200, 200, 200, 180),
            0.0f,
            0,
            1.0f
        );
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::drawDeathScreen() {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screenCenter = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

    ImGui::PushFont(bigFont);

    const char* text = "Press U to spawn";

    // Calculate text size
    ImVec2 textSize = ImGui::CalcTextSize(text);

    // Position cursor so text is centered
    ImGui::SetNextWindowPos(
        ImVec2(screenCenter.x - textSize.x * 0.5f,
            screenCenter.y - textSize.y * 0.5f),
        ImGuiCond_Always
    );

    // Invisible window
    ImGui::Begin(
        "##SpawnHint",
        nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_AlwaysAutoResize
    );

    ImGui::TextUnformatted(text);

    ImGui::PopFont();

    ImGui::End();
}

void GUI::drawServerBrowser() {
    ImGuiIO& io = ImGui::GetIO();

    const ImVec2 windowSize(640.0f, 520.0f);
    const ImVec2 windowPos(
        (io.DisplaySize.x - windowSize.x) * 0.5f,
        (io.DisplaySize.y - windowSize.y) * 0.5f
    );

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.95f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoMove;

    ImGui::Begin("Server Browser", nullptr, flags);

    ImGui::TextWrapped("Select a server to join or enter a custom host and port.");
    ImGui::Spacing();

    if (ImGui::Button("Refresh Servers")) {
        serverBrowserRefreshRequested = true;
    }
    ImGui::SameLine();
    ImVec4 scanColor = serverBrowserScanning ?
        ImVec4(0.60f, 0.80f, 0.40f, 1.0f) :
        ImVec4(0.70f, 0.70f, 0.70f, 1.0f);
    const char* scanLabel = serverBrowserScanning ? "Scanning..." : "Idle";
    ImGui::TextColored(scanColor, "%s", scanLabel);
    ImGui::Spacing();

    const ImGuiTableFlags tableFlags =
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_BordersOuter |
        ImGuiTableFlags_ScrollY;

    const float tableHeight = 260.0f;

    if (serverBrowserEntries.empty()) {
        ImGui::TextDisabled("No saved servers yet.");
    } else if (ImGui::BeginTable("##ServerBrowserPresets", 4, tableFlags, ImVec2(-1.0f, tableHeight))) {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None, 0.35f);
        ImGui::TableSetupColumn("Host", ImGuiTableColumnFlags_None, 0.30f);
        ImGui::TableSetupColumn("Port", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("Notes");
        ImGui::TableHeadersRow();

        for (int i = 0; i < static_cast<int>(serverBrowserEntries.size()); ++i) {
            const auto &entry = serverBrowserEntries[i];
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            bool selected = (serverBrowserSelectedIndex == i);
            std::string selectableLabel = entry.label + "##server_row_" + std::to_string(i);
            if (ImGui::Selectable(selectableLabel.c_str(), selected,
                    ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
                serverBrowserSelectedIndex = i;
                if (ImGui::IsMouseDoubleClicked(0)) {
                    pendingServerBrowserSelection = ServerBrowserSelection{ entry.host, entry.port, true };
                    serverBrowserStatusText.clear();
                    serverBrowserStatusIsError = false;
                }
            }

            ImGui::TableSetColumnIndex(1);
            const std::string &displayHost = entry.displayHost.empty() ? entry.host : entry.displayHost;
            ImGui::TextUnformatted(displayHost.c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%u", entry.port);

            ImGui::TableSetColumnIndex(3);
            ImGui::TextUnformatted(entry.description.c_str());
        }

        ImGui::EndTable();
    }

    if (ImGui::Button("Join Selected")) {
        if (serverBrowserSelectedIndex >= 0 &&
            serverBrowserSelectedIndex < static_cast<int>(serverBrowserEntries.size())) {
            const auto &entry = serverBrowserEntries[serverBrowserSelectedIndex];
            pendingServerBrowserSelection = ServerBrowserSelection{ entry.host, entry.port, true };
            serverBrowserStatusText.clear();
            serverBrowserStatusIsError = false;
        } else {
            serverBrowserStatusText = "Choose a server from the list first.";
            serverBrowserStatusIsError = true;
        }
    }

    ImGui::SameLine();
    ImGui::TextDisabled("Tip: double-click a row to quick-join");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Custom server");
    ImGui::InputText("Hostname", serverBrowserHostBuffer.data(), serverBrowserHostBuffer.size());
    ImGui::SetNextItemWidth(120.0f);
    ImGui::InputText("Port", serverBrowserPortBuffer.data(), serverBrowserPortBuffer.size(), ImGuiInputTextFlags_CharsDecimal);

    if (ImGui::Button("Join Custom")) {
        std::string hostValue(serverBrowserHostBuffer.data());
        if (hostValue.empty()) {
            serverBrowserStatusText = "Enter a hostname before joining.";
            serverBrowserStatusIsError = true;
        } else {
            std::string portString(serverBrowserPortBuffer.data());
            if (portString.empty()) {
                serverBrowserStatusText = "Enter a port before joining.";
                serverBrowserStatusIsError = true;
            } else {
                try {
                    int portValue = std::stoi(portString);
                    if (portValue < 1 || portValue > 65535) {
                        serverBrowserStatusText = "Ports must be between 1 and 65535.";
                        serverBrowserStatusIsError = true;
                    } else {
                        pendingServerBrowserSelection = ServerBrowserSelection{
                            hostValue,
                            static_cast<uint16_t>(portValue),
                            false
                        };
                        serverBrowserStatusText.clear();
                        serverBrowserStatusIsError = false;
                    }
                } catch (...) {
                    serverBrowserStatusText = "Port must be a valid number.";
                    serverBrowserStatusIsError = true;
                }
            }
        }
    }

    if (!serverBrowserStatusText.empty()) {
        ImGui::Spacing();
        ImVec4 color = serverBrowserStatusIsError ?
            ImVec4(0.93f, 0.36f, 0.36f, 1.0f) :
            ImVec4(0.60f, 0.80f, 0.40f, 1.0f);
        ImGui::TextColored(color, "%s", serverBrowserStatusText.c_str());
    }

    ImGui::End();
}

void GUI::resetServerBrowserBuffers(const std::string &defaultHost, uint16_t defaultPort) {
    std::string hostValue = defaultHost.empty() ? std::string("localhost") : defaultHost;
    uint16_t portValue = defaultPort == 0 ? 1234 : defaultPort;

    serverBrowserHostBuffer.fill(0);
    serverBrowserPortBuffer.fill(0);

    std::snprintf(serverBrowserHostBuffer.data(), serverBrowserHostBuffer.size(), "%s", hostValue.c_str());
    std::snprintf(serverBrowserPortBuffer.data(), serverBrowserPortBuffer.size(), "%u", portValue);
}

void GUI::showServerBrowser(const std::vector<ServerBrowserEntry> &entries,
    const std::string &defaultHost,
    uint16_t defaultPort) {
    showServerBrowserFlag = true;
    setServerBrowserEntries(entries);
    pendingServerBrowserSelection.reset();
    serverBrowserStatusText = "Select a server to connect or enter your own.";
    serverBrowserStatusIsError = false;
    resetServerBrowserBuffers(defaultHost, defaultPort);
}

void GUI::setServerBrowserEntries(const std::vector<ServerBrowserEntry> &entries) {
    serverBrowserEntries = entries;
    if (serverBrowserEntries.empty()) {
        serverBrowserSelectedIndex = -1;
    } else if (serverBrowserSelectedIndex < 0) {
        serverBrowserSelectedIndex = 0;
    } else if (serverBrowserSelectedIndex >= static_cast<int>(serverBrowserEntries.size())) {
        serverBrowserSelectedIndex = static_cast<int>(serverBrowserEntries.size()) - 1;
    }
}

void GUI::hideServerBrowser() {
    showServerBrowserFlag = false;
    serverBrowserStatusText.clear();
    serverBrowserStatusIsError = false;
    pendingServerBrowserSelection.reset();
    serverBrowserRefreshRequested = false;
    serverBrowserScanning = false;
}

bool GUI::isServerBrowserVisible() const {
    return showServerBrowserFlag;
}

void GUI::setServerBrowserStatus(const std::string &statusText, bool isErrorMessage) {
    serverBrowserStatusText = statusText;
    serverBrowserStatusIsError = isErrorMessage;
}

std::optional<GUI::ServerBrowserSelection> GUI::consumeServerBrowserSelection() {
    if (!pendingServerBrowserSelection.has_value()) {
        return std::nullopt;
    }

    auto selection = pendingServerBrowserSelection;
    pendingServerBrowserSelection.reset();
    return selection;
}

bool GUI::consumeServerBrowserRefreshRequest() {
    if (!serverBrowserRefreshRequested) {
        return false;
    }
    serverBrowserRefreshRequested = false;
    return true;
}

void GUI::setServerBrowserScanning(bool scanning) {
    serverBrowserScanning = scanning;
}

void GUI::setScoreboardPlayerNames(const std::vector<std::string> &names) {
    scoreboardPlayerNames = names;
}

void GUI::drawConsolePanel() {
    ImGuiIO& io = ImGui::GetIO();
    ImGuiViewport* vp = ImGui::GetMainViewport();

    const float margin = 12.0f;
    const float panelHeight = 260.0f;     // tweak
    const float inputHeight = 34.0f;      // tweak

    ImVec2 vpPos  = vp->Pos;        // usually (0,0)
    ImVec2 vpSize = vp->Size;

    ImVec2 pos  = ImVec2(vpPos.x + margin, vpPos.y + vpSize.y - margin - panelHeight);
    ImVec2 size = ImVec2(vpSize.x - 2.0f * margin, panelHeight);

    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(size, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.70f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings;

    

    ImGui::Begin("##BottomConsole", nullptr, flags);

    // Split area: scroll region above, input hint below
    const float footer = inputHeight;
    ImGui::BeginChild("##ConsoleScroll",
                      ImVec2(0, -footer),
                      true,
                      ImGuiWindowFlags_HorizontalScrollbar);

    // Auto-scroll logic: keep at bottom unless user scrolls up.
    // Call this bool member variable in your GUI class (persist between frames):
    // bool consoleAutoScroll = true;
    // Or use a static for quick testing:
    static bool autoScroll = true;

    // If user scrolls up, disable auto-scroll until they return to bottom
    const float scrollY = ImGui::GetScrollY();
    const float scrollMaxY = ImGui::GetScrollMaxY();
    const float atBottomEpsilon = 2.0f;
    if (scrollMaxY > 0.0f) {
        if (scrollY < scrollMaxY - atBottomEpsilon) {
            autoScroll = false;
        } else {
            autoScroll = true;
        }
    }

    // Render lines
    // Replace consoleLines with your storage (old messages, console logs, etc.)
    for (const std::string& line : consoleLines) {
        ImGui::TextUnformatted(line.c_str());
    }

    // If auto-scroll, pin to bottom AFTER adding items
    if (autoScroll) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();

    // Footer hint / fake input box
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::InvisibleButton("##focus_sink", ImVec2(1, 1));

    if (chatFocus) { 
        ImGui::SetKeyboardFocusHere();
    } else {
        ImGui::SetKeyboardFocusHere(-1);
    }

    ImGui::PushItemWidth(-1);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.75f);
    bool submitted = ImGui::InputTextWithHint(
        "##ChatHint",
        "press T to type",
        chatInputBuffer.data(),
        chatInputBuffer.size(),
        ImGuiInputTextFlags_EnterReturnsTrue
    );

    // Escape handling (same frame)
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        chatInputBuffer.fill(0);
        submittedInputBuffer.clear();
        chatFocus = false;
        spdlog::trace("GUI::drawConsolePanel: Chat input cancelled with Escape.");
    }
    
    ImGui::PopStyleVar();
    ImGui::PopItemWidth();

    ImGui::End();

    if (submitted) {
        submittedInputBuffer = std::string(chatInputBuffer.data());
        chatInputBuffer.fill(0); 
        chatFocus = false;
        spdlog::trace("GUI::drawConsolePanel: Submitted chat input: {}", submittedInputBuffer);
    }
}

void GUI::addConsoleLine(const std::string &playerName, const std::string &line) {
    std::string fullLine = playerName + ": " + line;
    consoleLines.push_back(fullLine);
}

std::string GUI::getChatInputBuffer() const {
    return submittedInputBuffer;
}

void GUI::clearChatInputBuffer() {
    submittedInputBuffer.clear();
}

void GUI::focusChatInput() {
    chatFocus = true;
}

bool GUI::getChatInputFocus() const {
    return chatFocus;
}

void GUI::displayDeathScreen(bool show) {
    drawDeathScreenFlag = show;
}