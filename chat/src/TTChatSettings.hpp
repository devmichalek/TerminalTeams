#pragma once
#include "TTUtilsMessageQueue.hpp"

class TTChatSettings {
public:
    explicit TTChatSettings(int argc, char** argv);
    virtual ~TTChatSettings() {}
    TTChatSettings(const TTChatSettings&) = delete;
    TTChatSettings(TTChatSettings&&) = delete;
    TTChatSettings& operator=(const TTChatSettings&) = delete;
    TTChatSettings& operator=(TTChatSettings&&) = delete;
    virtual size_t getTerminalWidth() const { return mWidth; }
    virtual size_t getTerminalHeight() const { return mHeight; }
    virtual std::shared_ptr<TTUtilsMessageQueue> getPrimaryMessageQueue() const;
    virtual std::shared_ptr<TTUtilsMessageQueue> getSecondaryMessageQueue() const;
    double getRatio() const { return 0.7; }
private:
    size_t mWidth;
    size_t mHeight;
    std::string mMessageQueueName;
    static inline constexpr int MAX_ARGC = 4;
    static inline const std::string PRIMARY_POSTFIX{"-primary"};
    static inline const std::string SECONDARY_POSTFIX{"-secondary"};
};
