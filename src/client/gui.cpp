#include "imgui.h"
#include "gui.hpp"
#include "renderer.hpp"
#include "backends/imgui_impl_glfw.h"      // your platform backend
#include "backends/imgui_impl_opengl3.h"   // your renderer backend
#include "client.hpp"
#include "player.hpp"

GUI &GUI::GetInstance() {
    static GUI instance;
    return instance;
}

void GUI::Init() {
    // Initialize ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    // Set ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(Renderer::GetInstance().window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Load fonts
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
}

void GUI::Update() {
    // Update ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

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
    std::string players = Player::GetInstance().GetName() + "\n";
    for (Client *client : Client::clients) {
        players += client->GetName() + "\n";
    }

    ImGui::Text("%s", players.c_str());

    // End window
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::Close() {
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}