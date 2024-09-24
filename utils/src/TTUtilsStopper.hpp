#pragma once
#include <atomic>
#include <mutex>
#include <functional>

using TTUtilsStopperCallback = std::function<void()>;

class TTUtilsStopper final {
public:
    explicit TTUtilsStopper(TTUtilsStopperCallback callback = [](){}) :
        mStopped{false}, mStoppedCounter{0}, mCallback(callback) {}
    ~TTUtilsStopper() = default;
    TTUtilsStopper(const TTUtilsStopper&) = delete;
    TTUtilsStopper(TTUtilsStopper&&) = delete;
    TTUtilsStopper& operator=(const TTUtilsStopper&) = delete;
    TTUtilsStopper& operator=(TTUtilsStopper&&) = delete;
    void stop() {
        mStopped.store(true);
        ++mStoppedCounter;
        std::call_once(mStoppedOnce, mCallback);
    }
    operator bool() const {
        return mStopped.load();
    }
    operator size_t() const {
        return mStoppedCounter.load();
    }
private:
    std::atomic<bool> mStopped;
    std::atomic<size_t> mStoppedCounter;
    std::once_flag mStoppedOnce;
    TTUtilsStopperCallback mCallback;
};
