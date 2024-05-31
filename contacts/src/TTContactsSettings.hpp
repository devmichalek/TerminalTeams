#pragma once
#include "TTUtilsSharedMem.hpp"

class TTContactsSettings {
public:
    explicit TTContactsSettings(int argc, const char* const* argv);
    virtual ~TTContactsSettings() {}
    TTContactsSettings(const TTContactsSettings&) = delete;
    TTContactsSettings(TTContactsSettings&&) = delete;
    TTContactsSettings& operator=(const TTContactsSettings&) = delete;
    TTContactsSettings& operator=(TTContactsSettings&&) = delete;
    virtual size_t getTerminalWidth() const { return mWidth; }
    virtual size_t getTerminalHeight() const { return mHeight; }
    virtual std::shared_ptr<TTUtilsSharedMem> getSharedMemory() const;
    virtual std::string getNickname() const { return mNickname; }
    virtual std::string getIdentity() const { return mIdentity; }
    virtual std::string getIpAddress() const { return mIpAddress; }
    virtual std::string getPort() const { return mPort; }
private:
    size_t mWidth;
    size_t mHeight;
    std::string mSharedMemoryName;
    std::string mNickname;
    std::string mIdentity;
    std::string mIpAddress;
    std::string mPort;
    static inline constexpr int MAX_ARGC = 8;
};
