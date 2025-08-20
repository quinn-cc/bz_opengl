#include "runtime.hpp"

Runtime &Runtime::GetInstance() {
    static Runtime instance;
    return instance;
}

void Runtime::Init() {
    lastTime = std::chrono::high_resolution_clock::now();
}

void Runtime::Update() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = currentTime - lastTime;
    deltaTime = elapsed.count();
    lastTime = currentTime;
}

float Runtime::GetDeltaTime() {
    return deltaTime;
}

void Runtime::Close() {
    
}