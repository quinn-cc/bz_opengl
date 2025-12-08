#pragma once
#include "Render.hpp"
#include "input.hpp"

class GLFWUserPointer {
public:
    Render *render;
    Input *input;
};