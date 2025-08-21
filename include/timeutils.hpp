#pragma once
#include <chrono>

namespace TimeUtils {
    using time = std::chrono::time_point<std::chrono::system_clock>;
    using duration = std::chrono::duration<float>;

    inline float GetElapsedTime(time start, time end) {
        return std::chrono::duration<float>(end - start).count();
    }

    inline duration GetDuration(time start, time end) {
        return std::chrono::duration<float>(end - start);
    }

    inline time GetCurrentTime() {
        return std::chrono::system_clock::now();
    }
}
