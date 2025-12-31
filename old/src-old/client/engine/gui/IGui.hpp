#pragma once
#include <GLFW/glfw3.h>
#include "backends/imgui_impl_glfw.h"      // your platform backend
#include "backends/imgui_impl_opengl3.h"   // your renderer backend

namespace Gui {

    class IGui {
    public:
        virtual void update() = 0;
    };

}