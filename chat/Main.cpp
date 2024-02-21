#include "TTChatBuilder.hpp"
#include "TTChat.hpp"

int main(int argc, char** argv) {
    TTChatBuilder builder(argc, argv);
    auto chat = builder.create();
    chat->run();
    return 0;
}