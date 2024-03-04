#pragma once
#include <string>
#include <vector>

class TTTextBoxSettings {
public:
    explicit TTTextBoxSettings(int argc, char** argv);
    size_t getTerminalWidth() const { return mWidth; }
    size_t getTerminalHeight() const { return mHeight; }
    std::string getPipeName() const { return mPipeName; }

private:
    size_t mWidth;
    size_t mHeight;
    std::string mPipeName;
    static inline constexpr int MAX_ARGC = 4;
};
