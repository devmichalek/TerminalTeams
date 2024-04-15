#pragma once
#include "TTContactsConsumer.hpp"

class TTContactsSettings {
public:
    explicit TTContactsSettings(int argc, const char* const* argv);
    virtual size_t getTerminalWidth() const;
    virtual size_t getTerminalHeight() const;
    virtual std::unique_ptr<TTContactsConsumer> getConsumer() const;
private:
    size_t mWidth;
    size_t mHeight;
    std::string mSharedMemoryName;
    static inline constexpr int MAX_ARGC = 4;
};
