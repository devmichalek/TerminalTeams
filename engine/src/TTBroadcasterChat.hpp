#pragma once
#include "TTContactsHandler.hpp"
#include "TTChatHandler.hpp"
#include "TTNetworkInterface.hpp"
#include "TTBroadcasterChatIf.hpp"
#include "TerminalTeams.grpc.pb.h"
#include <grpcpp/grpcpp.h>

using tt::NeighborsChat;
using tt::TellRequest;
using tt::TellReply;
using tt::NarrateRequest;
using tt::NarrateReply;

class TTBroadcasterChat : public TTBroadcasterChatIf {
public:
    TTBroadcasterChat(TTContactsHandler& contactsHandler, TTChatHandler& chatHandler, TTNetworkInterface interface);
    virtual ~TTBroadcasterChat();
    TTBroadcasterChat(const TTBroadcasterChat&) = delete;
    TTBroadcasterChat(TTBroadcasterChat&&) = delete;
    TTBroadcasterChat& operator=(const TTBroadcasterChat&) = delete;
    TTBroadcasterChat& operator=(TTBroadcasterChat&&) = delete;
    // Main loop
    virtual void run();
    // Stops application
    virtual void stop();
    // Returns true if application is stopped
    virtual bool stopped() const;
    // Handles message (send)
    virtual bool handleSend(const std::string& message);
    // Handles message (receive)
    virtual bool handleReceive(const TTNarrateMessage& message) override;
    // Handles message (receive)
    virtual bool handleReceive(const TTNarrateMessages& messages) override;
    // Returns root nickname
    virtual std::string getIdentity() const override;
private:
    using UniqueStub = std::unique_ptr<NeighborsChat::Stub>;
    struct Neighbor {
        UniqueStub stub;
        std::deque<std::string> pendingMessages;
    };
    UniqueStub createStub(const std::string& ipAddressAndPort);
    bool sendTell(const Neighbor& neighbor);
    bool sendNarrate(const Neighbor& neighbor);
    std::atomic<bool> mStopped;
    TTContactsHandler& mContactsHandler;
    TTChatHandler& mChatHandler;
    TTNetworkInterface mInterface;
    std::mutex mNeighborsMutex;
    std::condition_variable mNeighborsCondition;
    std::map<size_t, Neighbor> mNeighbors;
    static inline const std::chrono::milliseconds mNeighborsTimeout{100};
};
