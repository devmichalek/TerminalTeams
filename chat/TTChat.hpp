#pragma once
#include "TTEmulator.hpp"
#include "TTChatMessage.hpp"

class TTChat {
public:
    explicit TTChat(const TTEmulator& emulator, double ratio = 0.7);
    void print(const TTChatMessage& message);
    void print(const TTChatMessages& messages);
    void clear();
private:
    const TTEmulator& mEmulator;
    size_t mSideWidth;
    std::string mBlankLine;
};

