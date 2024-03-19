#include "TTChat.hpp"

int main(int argc, char** argv) {
    TTChatSettings settings(argc, argv);
    auto chat = TTChat(settings.getTerminalWidth(), settings.getTerminalHeight());

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    return 0;
}