#pragma once
#include "TTUtilsSharedMem.hpp"

class TTContactsSettings {
public:
    explicit TTContactsSettings(int argc, const char* const* argv);
    virtual ~TTContactsSettings() {}
    TTContactsSettings(const TTContactsSettings&) = default;
    TTContactsSettings(TTContactsSettings&&) = default;
    TTContactsSettings& operator=(const TTContactsSettings&) = default;
    TTContactsSettings& operator=(TTContactsSettings&&) = default;
    [[nodiscard]] virtual size_t getTerminalWidth() const { return mWidth; }
    [[nodiscard]] virtual size_t getTerminalHeight() const { return mHeight; }
    [[nodiscard]] virtual std::shared_ptr<TTUtilsSharedMem> getSharedMemory() const;
protected:
    TTContactsSettings() = default;
private:
    size_t mWidth = 0;
    size_t mHeight = 0;
    std::string mSharedMemoryName;
    static inline constexpr int MAX_ARGC = 4;
};
