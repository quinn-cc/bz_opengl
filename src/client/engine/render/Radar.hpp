#pragma once
#include "IRender.hpp"

namespace Render {
    class Radar : public IRender {
    private:
        

    public:
        Radar(GLFWwindow &window) : IRender(window) {}

        void update() override {
            // Implementation of rendering the player
        }
    };
}