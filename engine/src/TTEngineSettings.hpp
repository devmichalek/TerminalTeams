#pragma once
#include "TTContactsSettings.hpp"
#include "TTChatSettings.hpp"
#include "TTTextBoxSettings.hpp"
#include "TTNetworkInterface.hpp"
#include <deque>

class TTEngineSettings {
public:
    explicit TTEngineSettings(int argc, const char* const* argv);
    virtual ~TTEngineSettings() = default;
    TTEngineSettings(const TTEngineSettings&) = delete;
    TTEngineSettings(TTEngineSettings&&) = delete;
    TTEngineSettings& operator=(const TTEngineSettings&) = delete;
    TTEngineSettings& operator=(TTEngineSettings&&) = delete;
    virtual const TTContactsSettings& getContactsSettings() const { return *mContactsSettings; }
    virtual const TTChatSettings& getChatSettings() const { return *mChatSettings; }
    virtual const TTTextBoxSettings& getTextBoxSettings() const { return *mTextBoxSettings; }
    virtual const TTNetworkInterface& getInterface() const { return mInterface; }
    virtual const std::deque<std::string>& getNeighbors() const { return mNeighbors; }
private:
    // Delegated settings
    std::unique_ptr<TTContactsSettings> mContactsSettings;
    std::unique_ptr<TTChatSettings> mChatSettings;
    std::unique_ptr<TTTextBoxSettings> mTextBoxSettings;
    // Other settings
    TTNetworkInterface mInterface;
    std::deque<std::string> mNeighbors;
    static inline constexpr int MIN_ARGC = 9;
};
