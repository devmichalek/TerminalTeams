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
    TTContactsHandlerEntry(const std::string& nickname, const std::string& identity, const std::string& ipAddressAndPort) :
        nickname(nickname), identity(identity),
        ipAddressAndPort(ipAddressAndPort), sentMessages(0), receivedMessages(0),
        state(TTContactsState::ACTIVE) {}
    ~TTContactsHandlerEntry() = default;
    TTContactsHandlerEntry(const TTContactsHandlerEntry&) = default;
    TTContactsHandlerEntry(TTContactsHandlerEntry&&) = default;
    TTContactsHandlerEntry& operator=(const TTContactsHandlerEntry&) = default;
    TTContactsHandlerEntry& operator=(TTContactsHandlerEntry&&) = default;
};

inline std::ostream& operator<<(std::ostream& os, const TTContactsHandlerEntry& rhs)
{
    os << "{";
    os << "nickname: " << rhs.nickname << ", ";
    os << "identity: " << rhs.identity << ", ";
    os << "IP address and port: " << rhs.ipAddressAndPort << ", ";
    os << "sent messages: " << rhs.sentMessages << ", ";
    os << "received messages: " << rhs.receivedMessages;
    os << "}";
    return os;
}

inline bool operator==(const TTContactsHandlerEntry& lhs, const TTContactsHandlerEntry& rhs) {
    bool result = (lhs.nickname == rhs.nickname);
    result &= (lhs.identity == rhs.identity);
    result &= (lhs.ipAddressAndPort == rhs.ipAddressAndPort);
    result &= (lhs.sentMessages == rhs.sentMessages);
    result &= (lhs.receivedMessages == rhs.receivedMessages);
    result &= (lhs.state == rhs.state);
    return result;
}
