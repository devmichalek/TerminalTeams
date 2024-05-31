#pragma once
#include "TTContactsStatus.hpp"
#include <string>

// Dictionary entry
struct TTContactsEntry final {
    std::string nickname;
    std::string identity;
    std::string ipAddressAndPort;
    size_t sentMessages;
    size_t receivedMessages;
    TTContactsStatus status;
    TTContactsEntry(std::string nickname, std::string identity, std::string ipAddressAndPort) :
        nickname(nickname), identity(identity),
        ipAddressAndPort(ipAddressAndPort), sentMessages(0), receivedMessages(0),
        status(TTContactsStatus::ACTIVE) {}
    ~TTContactsEntry() = default;
    TTContactsEntry(const TTContactsEntry&) = default;
    TTContactsEntry(TTContactsEntry&&) = default;
    TTContactsEntry& operator=(const TTContactsEntry&) = default;
    TTContactsEntry& operator=(TTContactsEntry&&) = default;
};
