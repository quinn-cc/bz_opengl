#pragma once
#include "renderer.hpp"
#include "input.hpp"

class GLFWUserPointer {
public:
    Renderer *renderer;
    Input *input;
};