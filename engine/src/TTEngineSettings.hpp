#pragma once
#include <string>

class TTEngineSettings {
public:
    explicit TTChatSettings(int argc, char** argv);
    std::string getContactsSharedName() const { return mContactsSharedName; }
    std::string getChatQueueName() const { return mChatQueueName; }
    std::string getTextboxPipeName() const { return mTextboxPipeName; }
    std::string getInterface() const { return mInterface; }
    std::string getIpAddress() const { return mIpAddress; }
    uint16_t getPort() const { return mPort; }
    const std::vector<std::string>& getNeighbors() const { return mNeighbors; }

private:
    std::string mContactsSharedName;
    std::string mChatQueueName;
    std::string mTextboxPipeName;
    std::string mInterface;
    std::string mIpAddress;
    uint16_t mPort;
    std::vector<std::string> mNeighbors;
    static inline constexpr int MIN_ARGC = 7;
};
