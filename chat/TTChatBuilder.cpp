#include "TTChatBuilder.hpp"
#include <string>
#include <sstream>
#include <iostream>

TTChatBuilder::TTChatBuilder(int argc, char** argv) {
    const std::string classNamePrefix = "TTChatBuilder: ";
    if (argc != MAX_ARGC) {
        throw std::runtime_error(classNamePrefix + "invalid number of arguments");
    }

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

std::unique_ptr<TTChat> TTChatBuilder::create() const {
    std::cout << mWidth << std::endl;
    std::cout << mHeight << std::endl;
    return std::make_unique<TTChat>(mWidth, mHeight);
}