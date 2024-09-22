#pragma once
#include "TTUtilsMessageQueue.hpp"

class TTChatSettings {
public:
    explicit TTChatSettings(int argc, const char* const* argv);
    virtual ~TTChatSettings() {}
    TTChatSettings(const TTChatSettings&) = default;
    TTChatSettings(TTChatSettings&&) = default;
    TTChatSettings& operator=(const TTChatSettings&) = default;
    TTChatSettings& operator=(TTChatSettings&&) = default;
    virtual size_t getTerminalWidth() const { return mWidth; }
    virtual size_t getTerminalHeight() const { return mHeight; }
    virtual std::shared_ptr<TTUtilsMessageQueue> getPrimaryMessageQueue() const;
    virtual std::shared_ptr<TTUtilsMessageQueue> getSecondaryMessageQueue() const;
    virtual double getRatio() const { return 0.7; }
protected:
    TTChatSettings() = default;
private:
    size_t mWidth = 0;
    size_t mHeight = 0;
    std::string mMessageQueueName;
    static inline constexpr int MAX_ARGC = 4;
    static inline const std::string PRIMARY_POSTFIX{"-primary"};
    static inline const std::string SECONDARY_POSTFIX{"-secondary"};
};
