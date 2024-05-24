#include "TTTextBoxSettings.hpp"
#include "TTTextBoxMessage.hpp"
#include "TTUtilsSyscall.hpp"
#include <sstream>
#include <iostream>

TTTextBoxSettings::TTTextBoxSettings(int argc, char** argv) {
    const std::string classNamePrefix = "TTTextBoxSettings: ";
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

    const std::string uniqueName = argv[3];
    mUniquePath = "/tmp/" + uniqueName + "-pipe";
}

std::shared_ptr<TTUtilsNamedPipe> TTTextBoxSettings::getNamedPipe() const {
    return std::make_shared<TTUtilsNamedPipe>(mUniquePath,
                                              sizeof(TTTextBoxMessage),
                                              std::make_shared<TTUtilsSyscall>());
}
