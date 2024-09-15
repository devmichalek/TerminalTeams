#include "TTBroadcasterDiscovery.hpp"
#include "TTContactsHandlerMock.hpp"
#include "TTChatHandlerMock.hpp"
#include "TTNeighborsStubMock.hpp"
#include "TTNeighborsDiscoveryStubMock.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::Test;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::InSequence;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::Matcher;
using ::testing::ByMove;

class TTBroadcasterDiscoveryTest : public Test {
protected:
    TTBroadcasterDiscoveryTest() : mInterface("eno1", "192.168.1.1", "1777") {
        // Nothing more to be done
    }
    ~TTBroadcasterDiscoveryTest() {
    }
    // Called after constructor, before each test
    virtual void SetUp() override {
        mContactsHandler = std::make_shared<TTContactsHandlerMock>();
        mChatHandler = std::make_shared<TTChatHandlerMock>();
        mNeighborsStub = std::make_shared<TTNeighborsStubMock>();
        mNeighborIdentityCounter = 100;
    }
    // Called before destructor, after each test
    virtual void TearDown() override {
        mContactsHandler.reset();
        mChatHandler.reset();
        mNeighborsStub.reset();
        mNeighbors.clear();
        mHostEntry.reset();
        mNeighborEntries.clear();
        mNeighborStats.clear();
    }

    void SetNeighborCall(const std::string& nickname, const std::string& identity, const std::string& ipAddressAndPort) {
        InSequence __;
        EXPECT_CALL(*mContactsHandler, get(identity))
            .Times(1)
            .WillOnce(Return(std::nullopt));
        EXPECT_CALL(*mContactsHandler, create(nickname, identity, ipAddressAndPort))
            .Times(1)
            .WillOnce(Return(true));
        EXPECT_CALL(*mContactsHandler, get(identity))
            .Times(1)
            .WillOnce(Return(std::optional<size_t>(mNeighborIdentityCounter)));
        EXPECT_CALL(*mChatHandler, create(mNeighborIdentityCounter))
            .Times(1)
            .WillOnce(Return(true));
        ++mNeighborIdentityCounter;
    }

    void SetNeighborEntry(const std::string& nickname,
        const std::string& identity,
        const std::string& ipAddressAndPort) {
        mNeighbors.push_back(ipAddressAndPort);
        mNeighborEntries.emplace(std::piecewise_construct,
            std::forward_as_tuple(ipAddressAndPort),
            std::forward_as_tuple(nickname, identity, ipAddressAndPort));
        mNeighborStats.emplace(std::piecewise_construct,
            std::forward_as_tuple(ipAddressAndPort),
            std::forward_as_tuple());
        // Greet request
        TTGreetRequest expectedGreetRequest(mHostEntry->nickname, mHostEntry->identity, mHostEntry->ipAddressAndPort);
        EXPECT_CALL(*mNeighborsStub, createDiscoveryStub(ipAddressAndPort))
            .Times(1)
            .WillOnce(Return(ByMove(std::make_unique<TTNeighborsDiscoveryStubMock>(ipAddressAndPort))));
        EXPECT_CALL(*mNeighborsStub, sendGreet(_, expectedGreetRequest))
            .Times(1)
            .WillOnce([&](auto& stub, const auto& rhs) {
                const auto requestIpAddressAndPort = reinterpret_cast<TTNeighborsDiscoveryStubMock&>(stub).ipAddressAndPort;
                const auto& entry = mNeighborEntries[requestIpAddressAndPort];
                auto& stats = mNeighborStats[requestIpAddressAndPort];
                stats.sendGreetCounter += 1;
                return TTGreetResponse(true, entry.nickname, entry.identity, entry.ipAddressAndPort);
            });
        // Heartbeat request
        TTHeartbeatRequest expectedHeartbeatRequest(mHostEntry->identity);
        EXPECT_CALL(*mNeighborsStub, sendHeartbeat(_, expectedHeartbeatRequest))
            .Times(AtLeast(1))
            .WillRepeatedly([&](auto& stub, const auto& rhs) {
                const auto requestIpAddressAndPort = reinterpret_cast<TTNeighborsDiscoveryStubMock&>(stub).ipAddressAndPort;
                const auto& entry = mNeighborEntries[requestIpAddressAndPort];
                auto& stats = mNeighborStats[requestIpAddressAndPort];
                stats.sendHeartbeatCounter += 1;
                return TTHeartbeatResponse(true, entry.identity);
            });
        EXPECT_CALL(*mContactsHandler, activate(mNeighborIdentityCounter))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(true));
        // Add neighbor
        SetNeighborCall(nickname, identity, ipAddressAndPort);
        // Get neighbor
        std::optional<TTContactsHandlerEntry> entryOpt = {mNeighborEntries[ipAddressAndPort]};
        EXPECT_CALL(*mContactsHandler, get(Matcher<size_t>(size_t(mNeighborIdentityCounter))))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(entryOpt));
    }

    void SetHostEntry(const std::string& nickname, const std::string& identity, const std::string& ipAddressAndPort) {
        mHostEntry = std::make_unique<TTContactsHandlerEntry>(nickname, identity, ipAddressAndPort);
        std::optional<TTContactsHandlerEntry> entryOpt = {*mHostEntry};
        EXPECT_CALL(*mContactsHandler, get(Matcher<size_t>(size_t(0))))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(entryOpt));
    }

    struct NeighborStats {
        size_t sendGreetCounter = 0;
        size_t sendHeartbeatCounter = 0;
    };

    TTNetworkInterface mInterface;
    std::shared_ptr<TTContactsHandlerMock> mContactsHandler;
    std::shared_ptr<TTChatHandlerMock> mChatHandler;
    std::shared_ptr<TTNeighborsStubMock> mNeighborsStub;
    std::deque<std::string> mNeighbors;
    std::unique_ptr<TTContactsHandlerEntry> mHostEntry;
    std::map<std::string, TTContactsHandlerEntry> mNeighborEntries;
    size_t mNeighborIdentityCounter;
    std::map<std::string, NeighborStats> mNeighborStats;
};

TEST_F(TTBroadcasterDiscoveryTest, HappyPathReceiveGreetRequestNewNeighbor) {
    TTGreetRequest request("John", "5e5fe55f", "192.168.1.74:58");
    SetNeighborCall(request.nickname, request.identity, request.ipAddressAndPort);
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface, mNeighbors);
    EXPECT_TRUE(broadcaster->handleGreet(request));
    EXPECT_FALSE(broadcaster->stopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->stopped());
}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathReceiveGreetRequestExistingNeighbor) {
    TTGreetRequest request("John", "5e5fe55f", "192.168.1.74:58");
    std::optional<size_t> existingIdentity = 1;
    {
        InSequence __;
        EXPECT_CALL(*mContactsHandler, get(request.identity))
            .Times(1)
            .WillOnce(Return(existingIdentity));
        EXPECT_CALL(*mContactsHandler, activate(existingIdentity.value()))
            .Times(1)
            .WillOnce(Return(true));
    }
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface, mNeighbors);
    EXPECT_TRUE(broadcaster->handleGreet(request));
    EXPECT_FALSE(broadcaster->stopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->stopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathReceiveGreetRequestNewNeighborNoNickname) {
    TTGreetRequest request("", "5e5fe55f", "192.168.1.74:58");
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface, mNeighbors);
    EXPECT_FALSE(broadcaster->handleGreet(request));
    EXPECT_FALSE(broadcaster->stopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->stopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathReceiveGreetRequestNewNeighborNoIdentity) {
    TTGreetRequest request("John", "", "192.168.1.74:58");
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface, mNeighbors);
    EXPECT_FALSE(broadcaster->handleGreet(request));
    EXPECT_FALSE(broadcaster->stopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->stopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathReceiveGreetRequestNewNeighborNoIpAddressAndPort) {
    TTGreetRequest request("John", "5e5fe55f", "");
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface, mNeighbors);
    EXPECT_FALSE(broadcaster->handleGreet(request));
    EXPECT_FALSE(broadcaster->stopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->stopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathReceiveGreetRequestNewNeighborContactsHandlerFailedToCreate) {
    TTGreetRequest request("John", "5e5fe55f", "192.168.1.74:58");
    std::optional<size_t> newIdentity = 1;
    {
        InSequence __;
        EXPECT_CALL(*mContactsHandler, get(request.identity))
            .Times(1)
            .WillOnce(Return(std::nullopt));
        EXPECT_CALL(*mContactsHandler, create(request.nickname, request.identity, request.ipAddressAndPort))
            .Times(1)
            .WillOnce(Return(false));
    }
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface, mNeighbors);
    EXPECT_FALSE(broadcaster->handleGreet(request));
    EXPECT_FALSE(broadcaster->stopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->stopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathReceiveGreetRequestNewNeighborContactsHandlerFailedToGet) {
    TTGreetRequest request("John", "5e5fe55f", "192.168.1.74:58");
    std::optional<size_t> newIdentity = 1;
    {
        InSequence __;
        EXPECT_CALL(*mContactsHandler, get(request.identity))
            .Times(1)
            .WillOnce(Return(std::nullopt));
        EXPECT_CALL(*mContactsHandler, create(request.nickname, request.identity, request.ipAddressAndPort))
            .Times(1)
            .WillOnce(Return(true));
        EXPECT_CALL(*mContactsHandler, get(request.identity))
            .Times(1)
            .WillOnce(Return(std::nullopt));
    }
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface, mNeighbors);
    EXPECT_FALSE(broadcaster->handleGreet(request));
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->stopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathReceiveGreetRequestNewNeighborChatHandlerFailedToCreate) {
    TTGreetRequest request("John", "5e5fe55f", "192.168.1.74:58");
    std::optional<size_t> newIdentity = 1;
    {
        InSequence __;
        EXPECT_CALL(*mContactsHandler, get(request.identity))
            .Times(1)
            .WillOnce(Return(std::nullopt));
        EXPECT_CALL(*mContactsHandler, create(request.nickname, request.identity, request.ipAddressAndPort))
            .Times(1)
            .WillOnce(Return(true));
        EXPECT_CALL(*mContactsHandler, get(request.identity))
            .Times(1)
            .WillOnce(Return(newIdentity));
        EXPECT_CALL(*mChatHandler, create(newIdentity.value()))
            .Times(1)
            .WillOnce(Return(false));
    }
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface, mNeighbors);
    EXPECT_FALSE(broadcaster->handleGreet(request));
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->stopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathReceiveGreetRequestExistingNeighborContactsHandlerFailedToActivate) {
    TTGreetRequest request("John", "5e5fe55f", "192.168.1.74:58");
    std::optional<size_t> existingIdentity = 1;
    {
        InSequence __;
        EXPECT_CALL(*mContactsHandler, get(request.identity))
            .Times(1)
            .WillOnce(Return(existingIdentity));
        EXPECT_CALL(*mContactsHandler, activate(existingIdentity.value()))
            .Times(1)
            .WillOnce(Return(false));
    }
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface, mNeighbors);
    EXPECT_FALSE(broadcaster->handleGreet(request));
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->stopped());
}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathReceiveHeartbeatRequestExistingNeighbor) {
    TTHeartbeatRequest request("5e5fe55f");
    std::optional<size_t> existingIdentity = 1;
    {
        InSequence __;
        EXPECT_CALL(*mContactsHandler, get(request.identity))
            .Times(1)
            .WillOnce(Return(existingIdentity));
        EXPECT_CALL(*mContactsHandler, activate(existingIdentity.value()))
            .Times(1)
            .WillOnce(Return(true));
    }
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface, mNeighbors);
    EXPECT_TRUE(broadcaster->handleHeartbeat(request));
    EXPECT_FALSE(broadcaster->stopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->stopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathReceiveHeartbeatRequestNonExistingNeighbor) {
    TTHeartbeatRequest request("5e5fe55f");
    EXPECT_CALL(*mContactsHandler, get(request.identity))
        .Times(1)
        .WillOnce(Return(std::nullopt));
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface, mNeighbors);
    EXPECT_FALSE(broadcaster->handleHeartbeat(request));
    EXPECT_FALSE(broadcaster->stopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->stopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathReceiveHeartbeatRequestExistingNeighborContactsHandlerFailedToActivate) {
    TTHeartbeatRequest request("5e5fe55f");
    std::optional<size_t> existingIdentity = 1;
    {
        InSequence __;
        EXPECT_CALL(*mContactsHandler, get(request.identity))
            .Times(1)
            .WillOnce(Return(existingIdentity));
        EXPECT_CALL(*mContactsHandler, activate(existingIdentity.value()))
            .Times(1)
            .WillOnce(Return(false));
    }
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface, mNeighbors);
    EXPECT_FALSE(broadcaster->handleHeartbeat(request));
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->stopped());
}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathGetNickname) {
    SetHostEntry("Gabrielle", "6e6e6e6", "192.168.1.74:58");
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface, mNeighbors);
    EXPECT_EQ(broadcaster->getNickname(), mHostEntry->nickname);
    EXPECT_FALSE(broadcaster->stopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->stopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathGetNickname) {
    EXPECT_CALL(*mContactsHandler, get(Matcher<size_t>(size_t(0))))
        .Times(1)
        .WillOnce(Return(std::nullopt));
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface, mNeighbors);
    EXPECT_EQ(broadcaster->getNickname(), "");
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->stopped());
}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathGetIdentity) {
    SetHostEntry("Gabrielle", "6e6e6e6", "192.168.1.74:58");
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface, mNeighbors);
    EXPECT_EQ(broadcaster->getIdentity(), mHostEntry->identity);
    EXPECT_FALSE(broadcaster->stopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->stopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathGetIdentity) {
    EXPECT_CALL(*mContactsHandler, get(Matcher<size_t>(size_t(0))))
        .Times(1)
        .WillOnce(Return(std::nullopt));
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface, mNeighbors);
    EXPECT_EQ(broadcaster->getIdentity(), "");
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->stopped());
}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathGetIpAddressAndPort) {
    SetHostEntry("Gabrielle", "6e6e6e6", "192.168.1.74:58");
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface, mNeighbors);
    EXPECT_EQ(broadcaster->getIpAddressAndPort(), mHostEntry->ipAddressAndPort);
    EXPECT_FALSE(broadcaster->stopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->stopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathGetIpAddressAndPort) {
    EXPECT_CALL(*mContactsHandler, get(Matcher<size_t>(size_t(0))))
        .Times(1)
        .WillOnce(Return(std::nullopt));
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface, mNeighbors);
    EXPECT_EQ(broadcaster->getIpAddressAndPort(), "");
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->stopped());
}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathStaticNeighborsResolvedThenEachIsActive) {
    SetHostEntry("Gabrielle", "6e6e6e6", "192.168.1.74:58");
    SetNeighborEntry("Oak", "14eeffe", "122.124.0.9:55");
    SetNeighborEntry("Johny", "123456", "168.0.55.1:44");
    SetNeighborEntry("Camille", "ddddddd", "145.111.55.8:22");
    SetNeighborEntry("Steve", "9999999", "157.88.64.7:33");
    // Create broadcaster
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface, mNeighbors);
    // Start async consumer
    std::thread loop(std::bind(&TTBroadcasterDiscovery::run, broadcaster.get()));
    // Expect consumer to react
    std::this_thread::sleep_for(std::chrono::milliseconds{8000});
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    loop.join();

    for (const auto &[ipAddressAndPort, stats] : mNeighborStats) {
        EXPECT_EQ(stats.sendGreetCounter, 1);
        EXPECT_GE(stats.sendHeartbeatCounter, 1);
    }
}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathStaticNeighborsResolvedThenEachIsInactive) {

}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathStaticNeighborsResolvedThenEachIsFlaky) {

}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathStaticNeighborsUnresolved) {

}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathStaticNeighborsSomeResolvedThenEachIsActive) {

}
