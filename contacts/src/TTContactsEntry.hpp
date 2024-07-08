#pragma once
#include "TTContactsState.hpp"
#include <string>

// Dictionary entry
struct TTContactsEntry final {
    std::string nickname;
    std::string identity;
    std::string ipAddressAndPort;
    size_t sentMessages;
    size_t receivedMessages;
    TTContactsState state;
    TTContactsEntry(std::string nickname, std::string identity, std::string ipAddressAndPort) :
        nickname(nickname), identity(identity),
        ipAddressAndPort(ipAddressAndPort), sentMessages(0), receivedMessages(0),
        state(TTContactsState::ACTIVE) {}
    ~TTContactsEntry() = default;
    TTContactsEntry(const TTContactsEntry&) = default;
    TTContactsEntry(TTContactsEntry&&) = default;
    TTContactsEntry& operator=(const TTContactsEntry&) = default;
    TTContactsEntry& operator=(TTContactsEntry&&) = default;
};
