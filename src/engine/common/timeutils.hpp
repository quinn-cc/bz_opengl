#pragma once
#include <chrono>

namespace TimeUtils {
    using time = std::chrono::time_point<std::chrono::system_clock>;
    using duration = float;

    inline float GetElapsedTime(time start, time end) {
        return std::chrono::duration<float>(end - start).count();
    }

    inline time GetCurrentTime() {
        return std::chrono::system_clock::now();
    }
}
