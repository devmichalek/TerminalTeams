#pragma once
#include "TTChatMessage.hpp"

class TTChat {
public:
    explicit TTChat(size_t width, size_t height) :
        mWidth(width),
        mHeight(height),
        mSideWidth(width * SIDE_WIDTH_RATIO),
        mBlankLine(width, ' ') {}
    void print(const TTChatMessage& message);
    void print(const TTChatMessages& messages);
    void clear();
private:
    size_t mWidth;
    size_t mHeight;
    size_t mSideWidth;
    static inline constexpr double SIDE_WIDTH_RATIO = 0.7;
    std::string mBlankLine;
};

