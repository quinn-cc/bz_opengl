#pragma once
#include "GLFW/glfw3.h"
#include "backends/imgui_impl_glfw.h"      // your platform backend
#include "backends/imgui_impl_opengl3.h"   // your renderer backend
#include "Hud.hpp"

namespace Gui {

    class Gui {
    private:
        GLFWwindow &window;
        Hud hud;

    public:
        Gui(GLFWwindow &window) : window(window), hud() {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            
            // Set ImGui style
            ImGui::StyleColorsDark();

            // Setup Platform/Renderer bindings
            ImGui_ImplGlfw_InitForOpenGL(&window, true);
            ImGui_ImplOpenGL3_Init("#version 330");

            // Load fonts
            ImGuiIO& io = ImGui::GetIO();
            io.Fonts->AddFontDefault();
        }

        ~Gui() {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }

        void update() {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            hud.update();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
    };

}