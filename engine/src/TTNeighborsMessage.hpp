#pragma once
#include <string>
#include <deque>

struct TTTellRequest final {
    TTTellRequest(const std::string& identity, const std::string& message) : identity(identity), message(message) {}
    TTTellRequest() = default;
    ~TTTellRequest() = default;
    TTTellRequest(const TTTellRequest&) = default;
    TTTellRequest(TTTellRequest&&) = default;
    TTTellRequest& operator=(const TTTellRequest&) = default;
    TTTellRequest& operator=(TTTellRequest&&) = default;
    bool operator==(const TTTellRequest& rhs) const {
        return identity == rhs.identity && message == rhs.message;
    }
    std::string identity;
    std::string message;
};

struct TTTellResponse final {
    TTTellResponse(bool status) : status(status) {}
    TTTellResponse() = default;
    ~TTTellResponse() = default;
    TTTellResponse(const TTTellResponse&) = default;
    TTTellResponse(TTTellResponse&&) = default;
    TTTellResponse& operator=(const TTTellResponse&) = default;
    TTTellResponse& operator=(TTTellResponse&&) = default;
    bool status;
};

struct TTNarrateRequest final {
    TTNarrateRequest(const std::string& identity, const std::deque<std::string>& messages) :
        identity(identity), messages(messages) {}
    TTNarrateRequest() = default;
    ~TTNarrateRequest() = default;
    TTNarrateRequest(const TTNarrateRequest&) = default;
    TTNarrateRequest(TTNarrateRequest&&) = default;
    TTNarrateRequest& operator=(const TTNarrateRequest&) = default;
    TTNarrateRequest& operator=(TTNarrateRequest&&) = default;
    bool operator==(const TTNarrateRequest& rhs) const {
        return identity == rhs.identity && messages == rhs.messages;
    }
    std::string identity;
    std::deque<std::string> messages;
};

struct TTNarrateResponse final {
    TTNarrateResponse(bool status) : status(status) {}
    TTNarrateResponse() = default;
    ~TTNarrateResponse() = default;
    TTNarrateResponse(const TTNarrateResponse&) = default;
    TTNarrateResponse(TTNarrateResponse&&) = default;
    TTNarrateResponse& operator=(const TTNarrateResponse&) = default;
    TTNarrateResponse& operator=(TTNarrateResponse&&) = default;
    bool status;
};

struct TTGreetRequest final {
    TTGreetRequest(const std::string& nickname, const std::string& identity, const std::string& ipAddressAndPort) :
        nickname(nickname), identity(identity), ipAddressAndPort(ipAddressAndPort) {}
    TTGreetRequest() = default;
    ~TTGreetRequest() = default;
    TTGreetRequest(const TTGreetRequest&) = default;
    TTGreetRequest(TTGreetRequest&&) = default;
    TTGreetRequest& operator=(const TTGreetRequest&) = default;
    TTGreetRequest& operator=(TTGreetRequest&&) = default;
    bool operator==(const TTGreetRequest& rhs) const {
        return nickname == rhs.nickname && identity == rhs.identity && ipAddressAndPort == rhs.ipAddressAndPort;
    }
    std::string nickname;
    std::string identity;
    std::string ipAddressAndPort;
};

struct TTGreetResponse final {
    TTGreetResponse(bool status, const std::string& nickname, const std::string& identity, const std::string& ipAddressAndPort) :
        status(status), nickname(nickname), identity(identity), ipAddressAndPort(ipAddressAndPort) {}
    TTGreetResponse() = default;
    ~TTGreetResponse() = default;
    TTGreetResponse(const TTGreetResponse&) = default;
    TTGreetResponse(TTGreetResponse&&) = default;
    TTGreetResponse& operator=(const TTGreetResponse&) = default;
    TTGreetResponse& operator=(TTGreetResponse&&) = default;
    bool status;
    std::string nickname;
    std::string identity;
    std::string ipAddressAndPort;
};

struct TTHeartbeatRequest final {
    TTHeartbeatRequest(const std::string& identity) : identity(identity) {}
    TTHeartbeatRequest() = default;
    ~TTHeartbeatRequest() = default;
    TTHeartbeatRequest(const TTHeartbeatRequest&) = default;
    TTHeartbeatRequest(TTHeartbeatRequest&&) = default;
    TTHeartbeatRequest& operator=(const TTHeartbeatRequest&) = default;
    TTHeartbeatRequest& operator=(TTHeartbeatRequest&&) = default;
    bool operator==(const TTHeartbeatRequest& rhs) const {
        return identity == rhs.identity;
    }
    std::string identity;
};

struct TTHeartbeatResponse final {
    TTHeartbeatResponse(bool status, const std::string& identity) : status(status), identity(identity) {}
    TTHeartbeatResponse() = default;
    ~TTHeartbeatResponse() = default;
    TTHeartbeatResponse(const TTHeartbeatResponse&) = default;
    TTHeartbeatResponse(TTHeartbeatResponse&&) = default;
    TTHeartbeatResponse& operator=(const TTHeartbeatResponse&) = default;
    TTHeartbeatResponse& operator=(TTHeartbeatResponse&&) = default;
    bool status;
    std::string identity;
};
