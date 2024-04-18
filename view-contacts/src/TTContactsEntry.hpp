#pragma once
#include "TTContactsStatus.hpp"
#include <string>

const inline unsigned int TTCONTACTS_NICKNAME_MAX_LENGTH = 12;
const inline unsigned int TTCONTACTS_FULLNAME_MAX_LENGTH = 32;

// Dictionary entry
struct TTContactsEntry final {
    std::string nickname;
    std::string fullname;
    std::string ipAddressAndPort;
    size_t sentMessages;
    size_t receivedMessages;
    TTContactsStatus status;
    TTContactsEntry(std::string nickname, std::string fullname, std::string ipAddressAndPort) :
        nickname(nickname), fullname(fullname),
        ipAddressAndPort(ipAddressAndPort), sentMessages(0), receivedMessages(0),
        status(TTContactsStatus::ACTIVE) {}
    ~TTContactsEntry() = default;
    TTContactsEntry(const TTContactsEntry&) = default;
    TTContactsEntry(TTContactsEntry&&) = default;
    TTContactsEntry& operator=(const TTContactsEntry&) = default;
    TTContactsEntry& operator=(TTContactsEntry&&) = default;
};
