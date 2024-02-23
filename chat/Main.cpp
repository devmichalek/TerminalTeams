#include "TTEmulatorBuilder.hpp"
#include "TTChat.hpp"

#include <chrono>
#include <thread>

int main(int argc, char** argv) {
    TTEmulatorBuilder builder(argc, argv);
    auto emulator = builder.create();
    auto chat = TTChat(emulator);

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    return 0;
}