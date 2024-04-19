#pragma once
#include <string>

class TTChatSettings {
public:
    explicit TTChatSettings(int argc, char** argv);
    size_t getTerminalWidth() const { return mWidth; }
    size_t getTerminalHeight() const { return mHeight; }
    std::string getMessageQueueName() const { return mMessageQueueName; }
    static std::string getReversedMessageQueueName(std::string messageQueueName) {
        return messageQueueName + "-reversed";
    }
    double getRatio() { return 0.7; }
private:
    size_t mWidth;
    size_t mHeight;
    std::string mMessageQueueName;
    static inline constexpr int MAX_ARGC = 4;
};
