#pragma once
#include <chrono>

class TTTimestamp final {
public:
    TTTimestamp(std::chrono::milliseconds threshold) :
        mThreshold{threshold}, mTimestamp{std::chrono::steady_clock::now()} {}
    TTTimestamp() = default;
    ~TTTimestamp() = default;
    TTTimestamp(const TTTimestamp&) = default;
    TTTimestamp(TTTimestamp&&) = default;
    TTTimestamp& operator=(const TTTimestamp&) = default;
    TTTimestamp& operator=(TTTimestamp&&) = default;
    bool expired() const {
        return remaining() >= mThreshold;
    }
    void expire() {
        mTimestamp = std::chrono::time_point<std::chrono::steady_clock>(std::chrono::milliseconds(0));
    }
    std::chrono::milliseconds remaining() const {
        const auto end = std::chrono::steady_clock::now();
        const auto difference = std::chrono::duration_cast<std::chrono::milliseconds>(end - mTimestamp);
        return difference;
    }
    void kick() {
        mTimestamp = std::chrono::steady_clock::now();
    }
private:
    std::chrono::milliseconds mThreshold;
    std::chrono::time_point<std::chrono::steady_clock> mTimestamp;
};

