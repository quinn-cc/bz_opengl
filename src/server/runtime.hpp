#pragma once
#include <chrono>

class Runtime {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
    float deltaTime;

public:
    static Runtime &GetInstance();

    void Init();
    void Update();
    float GetDeltaTime();
    void Close();
};