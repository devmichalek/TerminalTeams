#pragma once
#include <string>

class TTTextBoxSettings {
public:
    explicit TTTextBoxSettings(int argc, char** argv);
    size_t getTerminalWidth() const { return mWidth; }
    size_t getTerminalHeight() const { return mHeight; }
    std::string getUniqueName() const { return mUniqueName; }
    static std::string getPipePath() { return "/tmp/" + mUniqueName + "-pipe"; }
    static std::string getSocketPath() { return "/tmp/" + mUniqueName + "-socket"; }

private:
    size_t mWidth;
    size_t mHeight;
    std::string mUniqueName;
    static inline constexpr int MAX_ARGC = 4;
};
