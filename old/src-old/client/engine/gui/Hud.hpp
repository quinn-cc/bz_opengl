#pragma once
#include "GLFW/glfw3.h"
#include "engine/gui/IGui.hpp"
#include <string>
#include "State.hpp"

namespace Gui {

    class Hud : public IGui {
    private:
        State::Game &game;

    public:
        Hud(State::Game &gameState) : game(gameState) {};

        void update() override {
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
            std::string players = "";
            for (auto user : game.users) {
                players += user->name + "\n";
            }

            ImGui::Text("%s", players.c_str());

            // End window
            ImGui::End();

            if (!game.player->alive) {
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
    };

}