#pragma once
#include "TTContactsState.hpp"
#include <string>

struct TTContactsEntry final {
    size_t identity;
    TTContactsState state;
    std::string nickname;
    TTContactsEntry(size_t identity, TTContactsState state, std::string nickname) :
        identity(identity), state(state), nickname(nickname) {}
    ~TTContactsEntry() = default;
    TTContactsEntry(const TTContactsEntry&) = default;
    TTContactsEntry(TTContactsEntry&&) = default;
    TTContactsEntry& operator=(const TTContactsEntry&) = default;
    TTContactsEntry& operator=(TTContactsEntry&&) = default;
};
