#pragma once
#include "TTEmulator.hpp"

class TTEmulatorBuilder {
public:
    explicit TTEmulatorBuilder(int argc, char** argv);
    TTEmulator create() const;

private:
    static inline constexpr size_t MAX_ARGC = 3;
    size_t mWidth;
    size_t mHeight;
};
