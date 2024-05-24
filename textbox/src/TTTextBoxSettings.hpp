#pragma once
#include "TTUtilsNamedPipe.hpp"

class TTTextBoxSettings {
public:
    explicit TTTextBoxSettings(int argc, char** argv);
    virtual ~TTTextBoxSettings() = default;
    TTTextBoxSettings(const TTTextBoxSettings&) = delete;
    TTTextBoxSettings(TTTextBoxSettings&&) = delete;
    TTTextBoxSettings& operator=(const TTTextBoxSettings&) = delete;
    TTTextBoxSettings& operator=(TTTextBoxSettings&&) = delete;
    virtual size_t getTerminalWidth() const { return mWidth; }
    virtual size_t getTerminalHeight() const { return mHeight; }
    virtual std::shared_ptr<TTUtilsNamedPipe> getNamedPipe() const;
protected:
    TTTextBoxSettings() = default;
private:
    size_t mWidth;
    size_t mHeight;
    std::string mUniquePath;
    static inline constexpr int MAX_ARGC = 4;
};
