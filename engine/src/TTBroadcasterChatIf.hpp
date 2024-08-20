#pragma once
#include <string>
#include <chrono>
#include <list>

struct TTNarrateMessage {
    std::string identity;
    std::string message;
};

using TTNarrateMessages = std::list<TTNarrateMessage>;

class TTBroadcasterChatIf {
public:
    TTBroadcasterChatIf() = default;
    virtual ~TTBroadcasterChatIf() = default;
    TTBroadcasterChatIf(const TTBroadcasterChatIf&) = default;
    TTBroadcasterChatIf(TTBroadcasterChatIf&&) = default;
    TTBroadcasterChatIf& operator=(const TTBroadcasterChatIf&) = default;
    TTBroadcasterChatIf& operator=(TTBroadcasterChatIf&&) = default;
    virtual bool handleTell(const TTNarrateMessage& message) = 0;
    virtual bool handleNarrate(const TTNarrateMessages& messages) = 0;
    virtual std::string getIdentity() const = 0;
};

