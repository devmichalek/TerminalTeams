#pragma once
#include "TTChatMessage.hpp"

class TTChat {
public:
    explicit TTChat(size_t width, size_t height, double ratio = 0.7);
    
private:
    void print(const TTChatMessage& message);
    void clear();
    size_t mWidth;
    size_t mHeight;
    size_t mSideWidth;
    std::string mBlankLine;
};

