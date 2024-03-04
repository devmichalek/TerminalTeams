#pragma once
#include <string>

class TTContactsSettings {
public:
    explicit TTContactsSettings(int argc, char** argv);
    size_t getTerminalWidth() const { return mWidth; }
    size_t getTerminalHeight() const { return mHeight; }
    std::string getSharedName() const { return mSharedName; }

private:
    size_t mWidth;
    size_t mHeight;
    std::string mSharedName;
    static inline constexpr int MAX_ARGC = 4;
};
