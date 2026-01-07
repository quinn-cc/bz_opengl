#include "engine/components/gui.hpp"
#include <GLFW/glfw3.h>
#include "spdlog/spdlog.h"
#include <cstdio>
#include <optional>
#include <algorithm>
#include <cctype>

namespace {
constexpr float kReferenceWindowWidth = 800.0f;
constexpr float kReferenceWindowHeight = 600.0f;

std::string trimCopy(const std::string &value) {
    auto begin = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
    auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    }).base();

    if (begin >= end) {
        return {};
    }

    return std::string(begin, end);
}

float calculateFontScale(const ImVec2 &displaySize) {
    if (displaySize.x <= 0.0f || displaySize.y <= 0.0f) {
        return 1.0f;
    }

    const float widthScale = displaySize.x / kReferenceWindowWidth;
    const float heightScale = displaySize.y / kReferenceWindowHeight;
    const float scaled = std::min(widthScale, heightScale);
    return std::max(1.0f, scaled);
}
}

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
        "../data/fonts/share_tech_mono_regular.ttf",
        100.0f   // font size in pixels
    );

    serverBrowserFont = io.Fonts->AddFontFromFileTTF(
        "../data/fonts/GoogleSans.ttf",
        20.0f
    );

    if (!serverBrowserFont) {
        spdlog::warn("Failed to load GoogleSans font for server browser.");
    }

    serverBrowserHeadingFont = io.Fonts->AddFontFromFileTTF(
        "../data/fonts/Audiowide.ttf",
        28.0f
    );

    if (!serverBrowserHeadingFont) {
        spdlog::warn("Failed to load Audiowide font for server browser headings.");
    }

    serverBrowserButtonFont = io.Fonts->AddFontFromFileTTF(
        "../data/fonts/Roboto.ttf",
        18.0f
    );

    if (!serverBrowserButtonFont) {
        spdlog::warn("Failed to load Roboto font for server browser buttons.");
    }

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
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = calculateFontScale(io.DisplaySize);
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
    const bool pushedServerBrowserFont = (serverBrowserFont != nullptr);
    if (pushedServerBrowserFont) {
        ImGui::PushFont(serverBrowserFont);
    }
    const bool hasHeadingFont = (serverBrowserHeadingFont != nullptr);
    const bool hasButtonFont = (serverBrowserButtonFont != nullptr);

    const ImVec2 windowSize(1200.0f, 680.0f);
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

    if (hasHeadingFont) {
        ImGui::PushFont(serverBrowserHeadingFont);
    }
    ImGui::Begin("Server Browser", nullptr, flags);
    if (hasHeadingFont) {
        ImGui::PopFont();
    }

    ImVec2 contentAvail = ImGui::GetContentRegionAvail();
    const ImGuiStyle &style = ImGui::GetStyle();
    const float minDetailWidth = 300.0f;
    const float minListWidth = 280.0f;
    float maxListWidth = std::max(minListWidth, contentAvail.x - minDetailWidth - style.ItemSpacing.x);
    float listPanelWidth = std::max(320.0f, contentAvail.x * 0.5f);
    listPanelWidth = std::clamp(listPanelWidth, minListWidth, maxListWidth);

    ImGui::BeginChild("ServerBrowserListPane", ImVec2(listPanelWidth, 0), false);

    auto formatListLabel = [](const ServerListOption &option) {
        if (!option.name.empty()) {
            return option.name;
        }
        if (!option.url.empty()) {
            return option.url;
        }
        return std::string("Unnamed list");
    };

    if (serverBrowserListOptions.empty() || serverBrowserListSelectedIndex < 0) {
        ImGui::TextDisabled("Add a server list below to fetch public servers.");
    } else {
        serverBrowserListSelectedIndex = std::clamp(
            serverBrowserListSelectedIndex,
            0,
            static_cast<int>(serverBrowserListOptions.size()) - 1);

        const auto &currentOption = serverBrowserListOptions[serverBrowserListSelectedIndex];
        std::string comboLabel = formatListLabel(currentOption);
        if (ImGui::BeginCombo("##ServerListSelector", comboLabel.c_str())) {
            for (int i = 0; i < static_cast<int>(serverBrowserListOptions.size()); ++i) {
                const auto &option = serverBrowserListOptions[i];
                std::string optionLabel = formatListLabel(option);
                bool selected = (i == serverBrowserListSelectedIndex);
                if (ImGui::Selectable(optionLabel.c_str(), selected)) {
                    if (!selected) {
                        serverBrowserListSelectedIndex = i;
                        pendingServerBrowserListSelection = i;
                    }
                }
                if (selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }

    ImGui::Spacing();

    ImVec4 scanColor = serverBrowserScanning ?
        ImVec4(0.60f, 0.80f, 0.40f, 1.0f) :
        ImVec4(0.70f, 0.70f, 0.70f, 1.0f);
    const char* scanLabel = serverBrowserScanning ? "Scanning..." : "Idle";
    const float refreshButtonWidth = ImGui::CalcTextSize("Refresh").x + style.FramePadding.x * 2.0f;
    const float statusTextWidth = ImGui::CalcTextSize(scanLabel).x;

    const ImGuiTableFlags tableFlags =
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_BordersOuter |
        ImGuiTableFlags_ScrollY;

    const float tableHeight = 260.0f;

    if (ImGui::BeginTable("##ServerBrowserPresets", 1, tableFlags, ImVec2(-1.0f, tableHeight))) {
        ImGui::TableSetupColumn("##ServerListColumn", ImGuiTableColumnFlags_None, 1.0f);

        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        ImGui::TableSetColumnIndex(0);

        const char* serversHeadingLabel = "Servers";
        ImVec2 headingTextSize;
        if (hasHeadingFont) {
            ImGui::PushFont(serverBrowserHeadingFont);
            headingTextSize = ImGui::CalcTextSize(serversHeadingLabel);
            ImGui::PopFont();
        } else {
            headingTextSize = ImGui::CalcTextSize(serversHeadingLabel);
        }

        const float headerStartX = ImGui::GetCursorPosX();
        const float headerStartY = ImGui::GetCursorPosY();
        const float headerWidth = ImGui::GetContentRegionAvail().x;
        float buttonX = headerStartX + headerWidth - refreshButtonWidth;
        float statusX = buttonX - style.ItemSpacing.x - statusTextWidth;
        const float minStatusX = headerStartX + headingTextSize.x + style.ItemSpacing.x;
        if (statusX < minStatusX) {
            statusX = minStatusX;
            buttonX = statusX + style.ItemSpacing.x + refreshButtonWidth;
        }

        const float maxButtonX = headerStartX + headerWidth - refreshButtonWidth;
        if (buttonX > maxButtonX) {
            buttonX = maxButtonX;
            statusX = std::max(headerStartX + headingTextSize.x + style.ItemSpacing.x,
                buttonX - style.ItemSpacing.x - statusTextWidth);
        }

        ImGui::SetCursorPos(ImVec2(headerStartX, headerStartY));
        if (hasHeadingFont) {
            ImGui::PushFont(serverBrowserHeadingFont);
        }
        ImGui::TextUnformatted(serversHeadingLabel);
        if (hasHeadingFont) {
            ImGui::PopFont();
        }
        float lineBottom = ImGui::GetCursorPosY();

        ImGui::SetCursorPos(ImVec2(statusX, headerStartY));
        ImGui::TextColored(scanColor, "%s", scanLabel);
        lineBottom = std::max(lineBottom, ImGui::GetCursorPosY());

        ImGui::SetCursorPos(ImVec2(buttonX, headerStartY));
        if (hasButtonFont) {
            ImGui::PushFont(serverBrowserButtonFont);
        }
        if (ImGui::Button("Refresh")) {
            serverBrowserRefreshRequested = true;
        }
        if (hasButtonFont) {
            ImGui::PopFont();
        }
        lineBottom = std::max(lineBottom, ImGui::GetCursorPosY());
        ImGui::SetCursorPosY(lineBottom);

        if (serverBrowserEntries.empty()) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextDisabled("No saved servers yet.");
        } else {
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
            }
        }

        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Custom server");
    ImGui::InputText("Address (host:port)", serverBrowserAddressBuffer.data(), serverBrowserAddressBuffer.size());

    bool joinCustomClicked = false;
    if (hasButtonFont) {
        ImGui::PushFont(serverBrowserButtonFont);
    }
    if (ImGui::Button("Join Custom")) {
        joinCustomClicked = true;
    }
    if (hasButtonFont) {
        ImGui::PopFont();
    }
    if (joinCustomClicked) {
        std::string addressValue = trimCopy(serverBrowserAddressBuffer.data());
        if (addressValue.empty()) {
            serverBrowserStatusText = "Enter a server address before joining.";
            serverBrowserStatusIsError = true;
        } else {
            auto colonPos = addressValue.find_last_of(':');
            if (colonPos == std::string::npos) {
                serverBrowserStatusText = "Use the format host:port (example: localhost:1234).";
                serverBrowserStatusIsError = true;
            } else {
                std::string hostPart = trimCopy(addressValue.substr(0, colonPos));
                std::string portPart = trimCopy(addressValue.substr(colonPos + 1));

                if (hostPart.empty()) {
                    serverBrowserStatusText = "Hostname cannot be empty.";
                    serverBrowserStatusIsError = true;
                } else if (portPart.empty()) {
                    serverBrowserStatusText = "Port cannot be empty.";
                    serverBrowserStatusIsError = true;
                } else {
                    try {
                        int portValue = std::stoi(portPart);
                        if (portValue < 1 || portValue > 65535) {
                            serverBrowserStatusText = "Ports must be between 1 and 65535.";
                            serverBrowserStatusIsError = true;
                        } else {
                            pendingServerBrowserSelection = ServerBrowserSelection{
                                hostPart,
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
    }

    if (!serverBrowserStatusText.empty()) {
        ImGui::Spacing();
        ImVec4 color = serverBrowserStatusIsError ?
            ImVec4(0.93f, 0.36f, 0.36f, 1.0f) :
            ImVec4(0.60f, 0.80f, 0.40f, 1.0f);
        ImGui::TextColored(color, "%s", serverBrowserStatusText.c_str());
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Add server list");
    ImGui::InputText("List URL", serverBrowserListUrlBuffer.data(), serverBrowserListUrlBuffer.size());

    bool saveListClicked = false;
    if (hasButtonFont) {
        ImGui::PushFont(serverBrowserButtonFont);
    }
    if (ImGui::Button("Save Server List")) {
        saveListClicked = true;
    }
    if (hasButtonFont) {
        ImGui::PopFont();
    }
    if (saveListClicked) {
        std::string urlValue(serverBrowserListUrlBuffer.data());
        if (urlValue.empty()) {
            serverBrowserListStatusText = "Enter a URL before saving.";
            serverBrowserListStatusIsError = true;
        } else {
            serverBrowserListStatusText.clear();
            serverBrowserListStatusIsError = false;
            pendingServerBrowserNewList = ServerListOption{ std::string{}, urlValue };
        }
    }

    if (!serverBrowserListStatusText.empty()) {
        ImGui::Spacing();
        ImVec4 listColor = serverBrowserListStatusIsError ?
            ImVec4(0.93f, 0.36f, 0.36f, 1.0f) :
            ImVec4(0.60f, 0.80f, 0.40f, 1.0f);
        ImGui::TextColored(listColor, "%s", serverBrowserListStatusText.c_str());
    }

    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("ServerBrowserDetailsPane", ImVec2(0, 0), true);
    if (hasHeadingFont) {
        ImGui::PushFont(serverBrowserHeadingFont);
    }
    ImGui::TextUnformatted("Server Details");
    if (hasHeadingFont) {
        ImGui::PopFont();
    }
    ImGui::SameLine();
    const float joinButtonWidth = ImGui::CalcTextSize("Join").x + style.FramePadding.x * 2.0f;
    const float joinButtonOffset = std::max(0.0f, ImGui::GetContentRegionAvail().x - joinButtonWidth);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + joinButtonOffset);
    bool joinSelectedClicked = false;
    if (hasButtonFont) {
        ImGui::PushFont(serverBrowserButtonFont);
    }
    if (ImGui::Button("Join")) {
        joinSelectedClicked = true;
    }
    if (hasButtonFont) {
        ImGui::PopFont();
    }
    if (joinSelectedClicked) {
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
    ImGui::Separator();

    const GUI::ServerBrowserEntry *selectedEntry = nullptr;
    if (serverBrowserSelectedIndex >= 0 &&
        serverBrowserSelectedIndex < static_cast<int>(serverBrowserEntries.size())) {
        selectedEntry = &serverBrowserEntries[serverBrowserSelectedIndex];
    }

    if (!selectedEntry) {
        ImGui::TextDisabled("Select a server to see more information.");
    } else {
        ImGui::TextWrapped("%s", selectedEntry->label.c_str());
        ImGui::Spacing();

        const std::string &displayHost = selectedEntry->displayHost.empty() ? selectedEntry->host : selectedEntry->displayHost;
        ImGui::Text("Host: %s", displayHost.c_str());
        ImGui::Text("Port: %u", selectedEntry->port);

        if (selectedEntry->activePlayers >= 0) {
            if (selectedEntry->maxPlayers >= 0) {
                ImGui::Text("Players: %d/%d", selectedEntry->activePlayers, selectedEntry->maxPlayers);
            } else {
                ImGui::Text("Players: %d", selectedEntry->activePlayers);
            }
        } else if (selectedEntry->maxPlayers >= 0) {
            ImGui::Text("Capacity: %d", selectedEntry->maxPlayers);
        }

        if (!selectedEntry->gameMode.empty()) {
            ImGui::Text("Mode: %s", selectedEntry->gameMode.c_str());
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("Description");
        if (!selectedEntry->longDescription.empty()) {
            ImGui::TextWrapped("%s", selectedEntry->longDescription.c_str());
        } else {
            ImGui::TextDisabled("No description provided.");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("Plugins");
        if (!selectedEntry->flags.empty()) {
            for (const auto &flag : selectedEntry->flags) {
                ImGui::BulletText("%s", flag.c_str());
            }
        } else {
            ImGui::TextDisabled("No plugins reported.");
        }
    }

    ImGui::EndChild();

    ImGui::End();

    if (pushedServerBrowserFont) {
        ImGui::PopFont();
    }
}

void GUI::resetServerBrowserBuffers(const std::string &defaultHost, uint16_t defaultPort) {
    std::string hostValue = defaultHost.empty() ? std::string("localhost") : defaultHost;
    uint16_t portValue = defaultPort == 0 ? 1234 : defaultPort;

    serverBrowserAddressBuffer.fill(0);
    std::snprintf(
        serverBrowserAddressBuffer.data(),
        serverBrowserAddressBuffer.size(),
        "%s:%u",
        hostValue.c_str(),
        portValue);
}

void GUI::showServerBrowser(const std::vector<ServerBrowserEntry> &entries,
    const std::string &defaultHost,
    uint16_t defaultPort) {
    showServerBrowserFlag = true;
    setServerBrowserEntries(entries);
    pendingServerBrowserSelection.reset();
    serverBrowserStatusText = "Select a server to connect or enter your own.";
    serverBrowserStatusIsError = false;
    pendingServerBrowserListSelection.reset();
    pendingServerBrowserNewList.reset();
    serverBrowserListStatusText.clear();
    serverBrowserListStatusIsError = false;
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

void GUI::setServerBrowserListOptions(const std::vector<ServerListOption> &options, int selectedIndex) {
    serverBrowserListOptions = options;
    if (serverBrowserListOptions.empty()) {
        serverBrowserListSelectedIndex = -1;
        pendingServerBrowserListSelection.reset();
        return;
    }

    if (selectedIndex < 0) {
        serverBrowserListSelectedIndex = 0;
    } else if (selectedIndex >= static_cast<int>(serverBrowserListOptions.size())) {
        serverBrowserListSelectedIndex = static_cast<int>(serverBrowserListOptions.size()) - 1;
    } else {
        serverBrowserListSelectedIndex = selectedIndex;
    }
}

std::optional<int> GUI::consumeServerBrowserListSelection() {
    if (!pendingServerBrowserListSelection.has_value()) {
        return std::nullopt;
    }

    auto selection = pendingServerBrowserListSelection;
    pendingServerBrowserListSelection.reset();
    return selection;
}

std::optional<GUI::ServerListOption> GUI::consumeServerBrowserNewListRequest() {
    if (!pendingServerBrowserNewList.has_value()) {
        return std::nullopt;
    }

    auto request = pendingServerBrowserNewList;
    pendingServerBrowserNewList.reset();
    return request;
}

void GUI::setServerBrowserListStatus(const std::string &statusText, bool isErrorMessage) {
    serverBrowserListStatusText = statusText;
    serverBrowserListStatusIsError = isErrorMessage;
}

void GUI::clearServerBrowserNewListInputs() {
    serverBrowserListUrlBuffer.fill(0);
}

void GUI::hideServerBrowser() {
    showServerBrowserFlag = false;
    serverBrowserStatusText.clear();
    serverBrowserStatusIsError = false;
    pendingServerBrowserSelection.reset();
    pendingServerBrowserListSelection.reset();
    pendingServerBrowserNewList.reset();
    serverBrowserRefreshRequested = false;
    serverBrowserScanning = false;
    serverBrowserListStatusText.clear();
    serverBrowserListStatusIsError = false;
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