#pragma once
#include "TTEmulator.hpp"

class TTEmulatorBuilder {
public:
    explicit TTEmulatorBuilder(int argc, char** argv);
    TTEmulator create() const;

private:
    size_t mWidth;
    size_t mHeight;
};
