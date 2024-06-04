#pragma once
#include <chrono>
#include <atomic>

class TTTimestamp {
public:
    explicit TTTimestamp(std::chrono::milliseconds threshold) :
            mThreshold{threshold}, mTimestamp{std::chrono::steady_clock::now()} {}
    ~TTTimestamp() = default;
    TTTimestamp(const TTTimestamp&) = delete;
    TTTimestamp(TTTimestamp&&) = delete;
    TTTimestamp& operator=(const TTTimestamp&) = delete;
    TTTimestamp& operator=(TTTimestamp&&) = delete;
    bool expired() const {
        const auto end = std::chrono::steady_clock::now();
        const auto difference = std::chrono::duration_cast<std::chrono::milliseconds>(mTimestamp.load() - end);
        return difference >= mThreshold.load();
    }
    void kick() {
        mTimestamp = std::chrono::steady_clock::now();
    }
private:
    std::atomic<std::chrono::milliseconds> mThreshold;
    std::atomic<std::chrono::time_point<std::chrono::steady_clock>> mTimestamp;
};

