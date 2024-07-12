#pragma once
#include <chrono>

class TTTimestamp final {
public:
    explicit TTTimestamp(std::chrono::milliseconds threshold) :
            mThreshold{threshold}, mTimestamp{std::chrono::steady_clock::now()} {}
    ~TTTimestamp() = default;
    TTTimestamp(const TTTimestamp&) = delete;
    TTTimestamp(TTTimestamp&&) = delete;
    TTTimestamp& operator=(const TTTimestamp&) = delete;
    TTTimestamp& operator=(TTTimestamp&&) = delete;
    bool expired() const {
        return remaining() >= mThreshold;
    }
    std::chrono::milliseconds remaining() const {
        const auto end = std::chrono::steady_clock::now();
        const auto difference = std::chrono::duration_cast<std::chrono::milliseconds>(mTimestamp - end);
        return difference;
    }
    void kick() {
        mTimestamp = std::chrono::steady_clock::now();
    }
private:
    std::chrono::milliseconds mThreshold;
    std::chrono::time_point<std::chrono::steady_clock> mTimestamp;
};

