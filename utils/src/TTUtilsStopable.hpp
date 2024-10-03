#pragma once
#include <atomic>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <deque>

class TTUtilsStopable {
public:
    TTUtilsStopable(const TTUtilsStopable&) = delete;
    TTUtilsStopable(TTUtilsStopable&&) = delete;
    TTUtilsStopable& operator=(const TTUtilsStopable&) = delete;
    TTUtilsStopable& operator=(TTUtilsStopable&&) = delete;
    // Thread-safe stop
    virtual void stop() {
        mStopped.store(true);
        std::call_once(mStoppedOnce, std::bind(&TTUtilsStopable::onStopInternal, this));
    }
    // Thread-safe getter
    [[nodiscard]] virtual bool isStopped() const {
        return mStopped.load();
    }
    // Thread-unsafe subscribe
    void subscribeOnStop(std::reference_wrapper<std::condition_variable> subscriber) {
        mStopSubscribers.emplace_back(subscriber);
    }
protected:
    TTUtilsStopable() :
        mStopped{false} {}
    ~TTUtilsStopable() = default;
    // Thread-safe on stop
    virtual void onStop() {}
private:
    void onStopInternal() {
        for (auto& subscriber : mStopSubscribers) {
            subscriber.get().notify_one();
        }
        onStop();
    }
    std::atomic<bool> mStopped;
    std::once_flag mStoppedOnce;
    std::deque<std::reference_wrapper<std::condition_variable>> mStopSubscribers;
};
