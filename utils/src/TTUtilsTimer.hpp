#pragma once
#include <chrono>

class TTUtilsTimer {
public:
    TTUtilsTimer(std::chrono::milliseconds threshold) :
        mThreshold{threshold}, mTimestamp{std::chrono::steady_clock::now()} {}
    TTUtilsTimer() = default;
    virtual ~TTUtilsTimer() = default;
    TTUtilsTimer(const TTUtilsTimer&) = default;
    TTUtilsTimer(TTUtilsTimer&&) = default;
    TTUtilsTimer& operator=(const TTUtilsTimer&) = default;
    TTUtilsTimer& operator=(TTUtilsTimer&&) = default;
    virtual bool expired() const {
        return remaining() >= mThreshold;
    }
    virtual void expire() {
        mTimestamp = std::chrono::time_point<std::chrono::steady_clock>(std::chrono::milliseconds(0));
    }
    virtual std::chrono::milliseconds remaining() const {
        const auto end = std::chrono::steady_clock::now();
        const auto difference = std::chrono::duration_cast<std::chrono::milliseconds>(end - mTimestamp);
        return difference;
    }
    virtual void kick() {
        mTimestamp = std::chrono::steady_clock::now();
    }
private:
    std::chrono::milliseconds mThreshold;
    std::chrono::time_point<std::chrono::steady_clock> mTimestamp;
};
