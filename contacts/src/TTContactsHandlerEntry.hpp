#pragma once
#include "TTContactsState.hpp"
#include <string>

struct TTContactsHandlerEntry final {
    std::string nickname;
    std::string identity;
    std::string ipAddressAndPort;
    size_t sentMessages;
    size_t receivedMessages;
    TTContactsState state;
    TTContactsHandlerEntry(std::string nickname, std::string identity, std::string ipAddressAndPort) :
        nickname(nickname), identity(identity),
        ipAddressAndPort(ipAddressAndPort), sentMessages(0), receivedMessages(0),
        state(TTContactsState::ACTIVE) {}
    ~TTContactsHandlerEntry() = default;
    TTContactsHandlerEntry(const TTContactsHandlerEntry&) = default;
    TTContactsHandlerEntry(TTContactsHandlerEntry&&) = default;
    TTContactsHandlerEntry& operator=(const TTContactsHandlerEntry&) = default;
    TTContactsHandlerEntry& operator=(TTContactsHandlerEntry&&) = default;
};