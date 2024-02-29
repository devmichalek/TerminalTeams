#include "TTChatSettings.hpp"
#include "TTEmulator.hpp"
#include "TTChat.hpp"

#include <chrono>
#include <thread>

int main(int argc, char** argv) {
    TTChatSettings settings(argc, argv);
    auto emulator = TTEmulator(settings.getTerminalWidth(), settings.getTerminalHeight());
    auto chat = TTChat(emulator);

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    return 0;
}