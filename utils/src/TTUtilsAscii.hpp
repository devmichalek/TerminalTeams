#pragma once
#include <array>

class TTUtilsAscii {
public:
    static inline bool isWhitespace(char c) {
        return std::find(std::begin(mWhitespaceChars), std::end(mWhitespaceChars), c) != mWhitespaceChars.end();
    }
private:
    constexpr static std::array<char, 6> mWhitespaceChars = {
        static_cast<char>(0x09),
        static_cast<char>(0x0A),
        static_cast<char>(0x0B),
        static_cast<char>(0x0C),
        static_cast<char>(0x0D),
        static_cast<char>(0x20)
    };
};
