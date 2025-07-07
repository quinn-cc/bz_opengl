#include "input.hpp"
#include "raylib.h"

Input &Input::GetInstance() {
    static Input instance;
    return instance;
}

Input::Input() {
    movement = { 0, 0 };
}

void Input::Update() {
    movement = { 0, 0 };
    fireReady = false;

    if (IsKeyDown(KEY_LEFT))
        movement.x -= 1;
    if (IsKeyDown(KEY_RIGHT))
        movement.x += 1;

    // Movement
    if (IsKeyDown(KEY_UP))
        movement.y += 1;
    if (IsKeyDown(KEY_DOWN))
        movement.y -= 1;

    // Fire
    if (IsKeyPressed(KEY_F)) {
        fireReady = true;
    }
}

bool Input::FireReady() {
    return fireReady;
}

glm::vec2 Input::GetMovement() {
    return movement;
}