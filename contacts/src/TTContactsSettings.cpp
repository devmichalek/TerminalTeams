#include "TTContactsSettings.hpp"
#include "TTContactsMessage.hpp"
#include "TTUtilsSyscall.hpp"
#include <string>
#include <sstream>

TTContactsSettings::TTContactsSettings(int argc, const char* const* argv) {
    if (argc != MAX_ARGC) {
        throw std::runtime_error(mClassNamePrefix + "invalid number of arguments");
    }

    const auto widthStr = std::string(argv[1]);
    std::stringstream ss(widthStr);
    ss >> mWidth;
    if (ss.fail()) {
        throw std::runtime_error(mClassNamePrefix + "invalid terminal emulator width=" + widthStr);
    }

    const auto heightStr = std::string(argv[2]);
    ss.str(heightStr);
    ss.clear();
    ss >> mHeight;
    if (ss.fail()) {
        throw std::runtime_error(mClassNamePrefix + "invalid terminal emulator height=" + heightStr);
    }

    mSharedMemoryName = argv[3];
}

size_t TTContactsSettings::getTerminalWidth() const {
    return mWidth;
}

size_t TTContactsSettings::getTerminalHeight() const {
    return mHeight;
}

std::shared_ptr<TTUtilsSharedMem> TTContactsSettings::getSharedMemory() const {
    const auto dataConsumedSemName = mSharedMemoryName + "-data-consumed";
    const auto dataProducedSemName = mSharedMemoryName + "-data-produced";
    return std::make_shared<TTUtilsSharedMem>(mSharedMemoryName,
                                              dataConsumedSemName,
                                              dataProducedSemName,
                                              sizeof(TTContactsMessage),
                                              std::make_shared<TTUtilsSyscall>());
}
