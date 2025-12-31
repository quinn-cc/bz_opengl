#include "imgui.h"
#include "gui.hpp"
#include "backends/imgui_impl_glfw.h"      // your platform backend
#include "backends/imgui_impl_opengl3.h"   // your renderer backend
#include "player.hpp"
#include "game.hpp"
#include "spdlog/spdlog.h"


void GUI::Init(Game *game, GLFWwindow* window) {
    this->game = game;

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
}

void GUI::StartFrame() {
    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::Update() {
    // ImGui::Begin("TopLeftText");
    ImGuiIO& io = ImGui::GetIO();

    // Set position and size
    ImGui::SetNextWindowPos(ImVec2(20, 20));       // top-left corner
    ImGui::SetNextWindowSize(ImVec2(500, 200));   // optional, just enough for your text
    ImGui::SetNextWindowBgAlpha(0.0f);           // make window background transparent

    // Begin window with no decorations
    ImGui::Begin("TopLeftText", nullptr, 
        ImGuiWindowFlags_NoTitleBar | 
        ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoScrollbar | 
        ImGuiWindowFlags_NoSavedSettings
    );

    // Add text
    std::string players = game->player.GetName() + "\n";
    for (auto client : game->clients) {
        players += client->GetName() + "\n";
    }

    ImGui::Text("%s", players.c_str());

    // End window
    ImGui::End();

    if (!game->player.IsAlive()) {
        ImGui::Begin("Centered Text", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove);
        // Get window size
        ImVec2 windowSize = ImGui::GetIO().DisplaySize;

        // Set cursor and draw
        ImGui::SetWindowFontScale(2.0f); // 2x bigger than normal
        const char * text = "Press 'u' to spawn.";
        // Get text size
        ImVec2 textSize(300, 100);

        // Calculate position for centered text
        ImVec2 textPos = ImVec2(
            (windowSize.x - textSize.x) * 0.5f,
            (windowSize.y - textSize.y) * 0.5f
        );
        ImGui::SetCursorScreenPos(textPos);

        ImGui::Text("%s", text);
        ImGui::SetWindowFontScale(1.0f);

        ImGui::End();
    }
}

void GUI::DrawRadar(unsigned int radarTextureId) {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoBackground |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoFocusOnAppearing;

    ImVec2 pos = ImVec2(RADAR_MARGIN, 
                        ImGui::GetIO().DisplaySize.y - RADAR_MARGIN - RADAR_SIZE);

    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(RADAR_SIZE, RADAR_SIZE));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0)); // remove padding
    ImGui::Begin("RadarOverlay", nullptr, flags);

    ImGui::Image((ImTextureID)(uintptr_t)radarTextureId,
                 ImVec2(RADAR_SIZE, RADAR_SIZE),
                 ImVec2(0, 1), ImVec2(1, 0));

    ImGui::End();
    ImGui::PopStyleVar();
}

void GUI::EndFrame() {
    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::Close() {
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}