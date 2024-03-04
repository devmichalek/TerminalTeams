#pragma once
#include "TTEmulator.hpp"

class TTTextBox {
public:
    explicit TTTextBox(const TTEmulator& emulator);
    void run();
private:
    const TTEmulator& mEmulator;
};

