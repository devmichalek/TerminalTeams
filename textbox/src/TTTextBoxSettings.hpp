#pragma once
#include "TTUtilsNamedPipe.hpp"

class TTTextBoxSettings {
public:
    explicit TTTextBoxSettings(int argc, const char* const* argv);
    virtual ~TTTextBoxSettings() = default;
    TTTextBoxSettings(const TTTextBoxSettings&) = default;
    TTTextBoxSettings(TTTextBoxSettings&&) = default;
    TTTextBoxSettings& operator=(const TTTextBoxSettings&) = default;
    TTTextBoxSettings& operator=(TTTextBoxSettings&&) = default;
    virtual size_t getTerminalWidth() const { return mWidth; }
    virtual size_t getTerminalHeight() const { return mHeight; }
    virtual std::shared_ptr<TTUtilsNamedPipe> getNamedPipe() const;
protected:
    TTTextBoxSettings() = default;
private:
    size_t mWidth = 0;
    size_t mHeight = 0;
    std::string mUniquePath;
    static inline constexpr int MAX_ARGC = 4;
};
