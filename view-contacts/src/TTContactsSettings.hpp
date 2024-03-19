#pragma once
#include <string>

class TTContactsSettings {
public:
    explicit TTContactsSettings(int argc, char** argv);
    size_t getTerminalWidth() const { return mWidth; }
    size_t getTerminalHeight() const { return mHeight; }
    std::string getSharedMemoryName() const { return mSharedMemoryName; }

private:
    size_t mWidth;
    size_t mHeight;
    std::string mSharedMemoryName;
    static inline constexpr int MAX_ARGC = 4;
};
