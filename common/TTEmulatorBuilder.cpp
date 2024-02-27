#include "TTEmulatorBuilder.hpp"
#include <string>
#include <sstream>
#include <iostream>

TTEmulatorBuilder::TTEmulatorBuilder(int argc, char** argv) {
    const std::string classNamePrefix = "TTEmulatorBuilder: ";
    const auto widthStr = std::string(argv[1]);
    std::stringstream ss(widthStr);
    ss >> mWidth;
    if (ss.fail()) {
        throw std::runtime_error(classNamePrefix + "invalid terminal emulator width=" + widthStr);
    }

    const auto heightStr = std::string(argv[2]);
    ss.str(heightStr);
    ss.clear();
    ss >> mHeight;
    if (ss.fail()) {
        throw std::runtime_error(classNamePrefix + "invalid terminal emulator height=" + heightStr);
    }
}

TTEmulator TTEmulatorBuilder::create() const {
    return TTEmulator(mWidth, mHeight);
}