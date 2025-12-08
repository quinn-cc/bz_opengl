#pragma once
#include <chrono>
#include <timeutils.hpp>

class Runtime {
private:
    TimeUtils::time lastTime;
    TimeUtils::duration deltaTime;

public:
    void Init() {
        lastTime = TimeUtils::GetCurrentTime();
    }

    void Update() {
        TimeUtils::time currentTime = TimeUtils::GetCurrentTime();
        deltaTime = TimeUtils::GetElapsedTime(lastTime, currentTime);
        lastTime = currentTime;
    }

    TimeUtils::duration GetDeltaTime() {
        return deltaTime;
    }

    void Close() {

    }
};