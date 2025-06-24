#include "player.hpp"
#include "raylib.h"

void Player::Draw() {
    DrawCube(position, 1, 1, 1, RED);
    DrawCubeWires(position, 1, 1, 1, BLACK);
}