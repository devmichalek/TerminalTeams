#pragma once
#include "TTAbstractFactory.hpp"
#include "TTContactsSettings.hpp"
#include "TTChatSettings.hpp"
#include "TTTextBoxSettings.hpp"
#include "TTNetworkInterface.hpp"

class TTEngineSettings {
public:
    explicit TTEngineSettings(int argc, const char* const* argv);
    virtual ~TTEngineSettings() = default;
    TTEngineSettings(const TTEngineSettings&) = delete;
    TTEngineSettings(TTEngineSettings&&) = delete;
    TTEngineSettings& operator=(const TTEngineSettings&) = delete;
    TTEngineSettings& operator=(TTEngineSettings&&) = delete;
    [[nodiscard]] virtual const std::string& getNickname() const { return mNickname; }
    [[nodiscard]] virtual const std::string& getIdentity() const { return mIdentity; }
    [[nodiscard]] virtual const TTNetworkInterface& getNetworkInterface() const { return mNetworkInterface; }
    [[nodiscard]] virtual const std::deque<std::string>& getNeighbors() const { return mNeighbors; }
    [[nodiscard]] virtual const TTAbstractFactory& getAbstractFactory() const { return *mAbstractFactory; }
protected:
    TTEngineSettings() = default;
private:
    // Delegated settings
    std::unique_ptr<TTContactsSettings> mContactsSettings;
    std::unique_ptr<TTChatSettings> mChatSettings;
    std::unique_ptr<TTTextBoxSettings> mTextBoxSettings;
    // Abstract factory
    std::unique_ptr<TTAbstractFactory> mAbstractFactory;
    // Other settings
    std::string mNickname;
    std::string mIdentity;
    TTNetworkInterface mNetworkInterface;
    std::deque<std::string> mNeighbors;
    static inline constexpr int MIN_ARGC = 9;
};
