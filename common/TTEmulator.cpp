#include "TTEmulator.hpp"
#include <iostream>

TTEmulator::TTEmulator(size_t width, size_t height) :
    mWidth(width), mHeight(height) {
}

void TTEmulator::print(std::string message) const {
    std::cout << message;
}

void TTEmulator::println(std::string message) const {
    std::cout << message << "\n";
}

void TTEmulator::flush() const {
    std::cout << std::flush;
}

void TTEmulator::clear() const {
    system("clear");
}

size_t TTEmulator::getWidth() const {
    return mWidth;
}

size_t TTEmulator::getHeight() const {
    return mHeight;
}