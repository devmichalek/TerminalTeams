#pragma once
#include <string>

class TTChatSettings {
public:
    explicit TTChatSettings(int argc, char** argv);
    size_t getTerminalWidth() const { return mWidth; }
    size_t getTerminalHeight() const { return mHeight; }
    std::string getQueueName() const { return mQueueName; }

private:
    size_t mWidth;
    size_t mHeight;
    std::string mQueueName;
    static inline constexpr int MAX_ARGC = 4;
};
