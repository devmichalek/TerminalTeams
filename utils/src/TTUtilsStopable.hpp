#pragma once
#include <atomic>
#include <mutex>
#include <functional>

class TTUtilsStopable {
public:
    TTUtilsStopable(const TTUtilsStopable&) = delete;
    TTUtilsStopable(TTUtilsStopable&&) = delete;
    TTUtilsStopable& operator=(const TTUtilsStopable&) = delete;
    TTUtilsStopable& operator=(TTUtilsStopable&&) = delete;
    // Thread-safe stop
    virtual void stop() {
        mStopped.store(true);
        std::call_once(mStoppedOnce, std::bind(&TTUtilsStopable::onStop, this));
    }
    // Thread-safe getter
    [[nodiscard]] virtual bool isStopped() const {
        return mStopped.load();
    }
protected:
    TTUtilsStopable() :
        mStopped{false} {}
    ~TTUtilsStopable() = default;
    // Thread-safe on stop
    virtual void onStop() {}
private:
    std::atomic<bool> mStopped;
    std::once_flag mStoppedOnce;
};
