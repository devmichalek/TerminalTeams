#include "TTChatSettings.hpp"
#include <charconv>

TTChatSettings::TTChatSettings(int argc, char** argv) {
    const std::string classNamePrefix = "TTChatSettings: ";
    if (argc != MAX_ARGC) {
        throw std::runtime_error(classNamePrefix + "invalid number of arguments");
    }

    {
        auto [ptr, ec] = std::from_chars(argv[1], argv[1] + strlen(argv[1]), mWidth);
        if (ec != std::errc()) {
            throw std::runtime_error(classNamePrefix + "invalid terminal emulator width=" + argv[1]);
        }
    }

    {
        auto [ptr, ec] = std::from_chars(argv[2], argv[2] + strlen(argv[2]), mWidth);
        if (ec != std::errc()) {
            throw std::runtime_error(classNamePrefix + "invalid terminal emulator height=" + argv[2]);
        }
    }

    mMessageQueueName = argv[3];
}
