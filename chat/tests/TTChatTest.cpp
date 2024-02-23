

// To do


#include "TTEmulatorBuilder.hpp"
#include "TTChat.hpp"

#include <chrono>
#include <thread>

int main(int argc, char** argv) {
    TTEmulatorBuilder builder(argc, argv);
    auto emulator = builder.create();
    auto chat = TTChat(emulator);

    const auto now = std::chrono::system_clock::now();
    auto m1 = TTChatMessage(TTChatSide::LEFT, now, "          Hello from the other side");
    auto m2 = TTChatMessage(TTChatSide::RIGHT, now, "Hello, hope    you  doing   well?      ");
    auto m3 = TTChatMessage(TTChatSide::RIGHT, now, "because I haven't hear anything from you for the long time...");
    auto m4 = TTChatMessage(TTChatSide::LEFT, now, "Yes, I'm ok                 ");
    auto m5 = TTChatMessage(TTChatSide::LEFT, now, "and you?");
    chat.print(m1);
    chat.print(m2);
    chat.print(m3);
    chat.print(m4);
    chat.print(m5);

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    return 0;
}