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
    virtual size_t getTerminalWidth() const;
    virtual size_t getTerminalHeight() const;
    virtual std::shared_ptr<TTUtilsSharedMem> getSharedMemory() const;
private:
    inline static const std::string mClassNamePrefix = "TTContactsSettings: ";
    size_t mWidth;
    size_t mHeight;
    std::string mSharedMemoryName;
    static inline constexpr int MAX_ARGC = 4;
};
