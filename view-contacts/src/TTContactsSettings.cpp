#include "TTContactsSettings.hpp"
#include "TTContactsMessage.hpp"
#include "TTContactsSyscall.hpp"
#include <string>
#include <sstream>
#include <iostream>

TTContactsSettings::TTContactsSettings(int argc, const char* const* argv) {
    const std::string classNamePrefix = "TTContactsSettings: ";
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

    mSharedMemoryName = argv[3];
}

size_t TTContactsSettings::getTerminalWidth() const {
    return mWidth;
}

size_t TTContactsSettings::getTerminalHeight() const {
    return mHeight;
    }

std::unique_ptr<TTContactsConsumer> TTContactsSettings::getConsumer() const {
    const auto dataConsumedSemName = mSharedMemoryName + TTCONTACTS_DATA_CONSUMED_POSTFIX;
    const auto dataProducedSemName = mSharedMemoryName + TTCONTACTS_DATA_PRODUCED_POSTFIX;
    return std::make_unique<TTContactsConsumer>(mSharedMemoryName,
                                                dataConsumedSemName,
                                                dataProducedSemName,
                                                std::make_shared<TTContactsSyscall>());
}
