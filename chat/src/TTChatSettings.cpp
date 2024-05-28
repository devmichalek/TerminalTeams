#include "TTChatSettings.hpp"
#include "TTChatMessage.hpp"
#include "TTUtilsSyscall.hpp"
#include <charconv>
#include <cstring>
#include <stdexcept>

TTChatSettings::TTChatSettings(int argc, const char* const* argv) {
    if (argc != MAX_ARGC) {
        throw std::runtime_error("TTChatSettings: Invalid number of arguments");
    }

    {
        auto [ptr, ec] = std::from_chars(argv[1], argv[1] + strlen(argv[1]), mWidth);
        if (ec != std::errc()) {
            throw std::runtime_error(std::string("TTChatSettings: Invalid terminal emulator width=") + argv[1]);
        }
    }

    {
        auto [ptr, ec] = std::from_chars(argv[2], argv[2] + strlen(argv[2]), mHeight);
        if (ec != std::errc()) {
            throw std::runtime_error(std::string("TTChatSettings: Invalid terminal emulator height=") + argv[2]);
        }
    }

    // Set queue name
    mMessageQueueName = argv[3];
    if (mMessageQueueName.front() != '/') {
        mMessageQueueName.insert(0, "/");
    }
}

std::shared_ptr<TTUtilsMessageQueue> TTChatSettings::getPrimaryMessageQueue() const {
    const auto queueName = mMessageQueueName + PRIMARY_POSTFIX;
    return std::make_shared<TTUtilsMessageQueue>(queueName, 8, sizeof(TTChatMessage), std::make_shared<TTUtilsSyscall>());
}

std::shared_ptr<TTUtilsMessageQueue> TTChatSettings::getSecondaryMessageQueue() const {
    const auto queueName = mMessageQueueName + SECONDARY_POSTFIX;
    return std::make_shared<TTUtilsMessageQueue>(queueName, 8, sizeof(TTChatMessage), std::make_shared<TTUtilsSyscall>());
}
