#pragma once
#include "TTUtilsTimer.hpp"
#include <random>

class TTUtilsTimerFactory {
public:
    TTUtilsTimerFactory(std::chrono::milliseconds minThreshold, std::chrono::milliseconds maxThreshold) :
        mRandomNumberGenerator(std::random_device{}()),
        mDistribution(minThreshold.count(), maxThreshold.count()) {}
    ~TTUtilsTimerFactory() = default;
    TTUtilsTimerFactory(const TTUtilsTimerFactory&) = default;
    TTUtilsTimerFactory(TTUtilsTimerFactory&&) = default;
    TTUtilsTimerFactory& operator=(const TTUtilsTimerFactory&) = default;
    TTUtilsTimerFactory& operator=(TTUtilsTimerFactory&&) = default;
    TTUtilsTimer create() {
        return TTUtilsTimer(std::chrono::milliseconds(mDistribution(mRandomNumberGenerator)));
    }
    std::chrono::milliseconds min() {
        return std::chrono::milliseconds(mDistribution.min());
    }
    std::chrono::milliseconds max() {
        return std::chrono::milliseconds(mDistribution.max());
    }
private:
    std::mt19937 mRandomNumberGenerator;
    std::uniform_int_distribution<std::mt19937::result_type> mDistribution;
    
};
