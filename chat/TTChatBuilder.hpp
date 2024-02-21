#pragma once
#include <memory>
#include "TTChat.hpp"

class TTChatBuilder {
public:
    explicit TTChatBuilder(int argc, char** argv);
    std::unique_ptr<TTChat> create() const;

private:
    static inline constexpr size_t MAX_ARGC = 3;
    size_t mWidth;
    size_t mHeight;
};
