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
    TTBroadcasterDiscoveryTest() : mNetworkInterface("eno1", "192.168.1.1", "1777") {
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
    }

    void SetNeighborsGreetCalls() {
        const TTGreetRequest expectedGreetRequest(mHostEntry->nickname, mHostEntry->identity, mHostEntry->ipAddressAndPort);
        EXPECT_CALL(*mNeighborsStub, sendGreet(_, expectedGreetRequest))
            .Times(AtLeast(1))
            .WillRepeatedly([&](auto& stub, const auto& rhs) {
                const auto requestIpAddressAndPort = reinterpret_cast<TTNeighborsDiscoveryStubMock&>(stub).ipAddressAndPort;
                auto& entry = mNeighborEntries.at(requestIpAddressAndPort);
                entry.sendGreetCounter += 1;
                const auto retcode = entry.greets.front();
                entry.greets.pop_front();
                return TTGreetResponse(retcode, entry.nickname, entry.identity, entry.ipAddress + ":" + entry.port);
            });
    }

    void SetNeighborsHeartbeatCalls() {
        const TTHeartbeatRequest expectedHeartbeatRequest(mHostEntry->identity);
        EXPECT_CALL(*mNeighborsStub, sendHeartbeat(_, expectedHeartbeatRequest))
            .Times(AtLeast(1))
            .WillRepeatedly([&](auto& stub, const auto& rhs) {
                const auto requestIpAddressAndPort = reinterpret_cast<TTNeighborsDiscoveryStubMock&>(stub).ipAddressAndPort;
                auto& entry = mNeighborEntries.at(requestIpAddressAndPort);
                entry.sendHeartbeatCounter += 1;
                const auto retcode = entry.heartbeats.front();
                entry.heartbeats.pop_front();
                return TTHeartbeatResponse(retcode, entry.identity);
            });
    }

    void SetNeighborCreateDiscoveryStub(const std::string& ipAddressAndPort) {
        EXPECT_CALL(*mNeighborsStub, createDiscoveryStub(ipAddressAndPort))
            .Times(1)
            .WillOnce(Return(ByMove(std::make_unique<TTNeighborsDiscoveryStubMock>(ipAddressAndPort))));
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
        const std::string& ipAddress,
        const std::deque<bool>& greets,
        const std::deque<bool>& heartbeats) {
        mNeighborEntries.emplace(std::piecewise_construct,
            std::forward_as_tuple(ipAddress + ":" + mNetworkInterface.getPort()),
            std::forward_as_tuple(nickname, identity, ipAddress, mNetworkInterface.getPort(), greets, heartbeats));
    }

    void SetNeighborEntryAndCall(const std::string& nickname,
        const std::string& identity,
        const std::string& ipAddress,
        const std::deque<bool>& greets,
        const std::deque<bool>& heartbeats) {
        SetNeighborEntry(nickname, identity, ipAddress, greets, heartbeats);
        InSequence __;
        for (const auto &i : greets) {
            SetNeighborCreateDiscoveryStub(ipAddress + ":" + mNetworkInterface.getPort());
        }
        SetNeighborCall(nickname, identity, ipAddress + ":" + mNetworkInterface.getPort());
    }

    void SetHostEntryAndCall(const std::string& nickname, const std::string& identity, const std::string& ipAddress) {
        mHostEntry = std::make_unique<TTContactsHandlerEntry>(nickname, identity, ipAddress + ":" + mNetworkInterface.getPort());
        std::optional<TTContactsHandlerEntry> entryOpt = {*mHostEntry};
        EXPECT_CALL(*mContactsHandler, get(Matcher<size_t>(size_t(0))))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(entryOpt));
    }

    struct NeighborEntry {
        NeighborEntry(const std::string& nickname,
            const std::string& identity,
            const std::string& ipAddress,
            const std::string& port,
            std::deque<bool> greets,
            std::deque<bool> heartbeats) :
            nickname(nickname), identity(identity), ipAddress(ipAddress), port(port), greets(greets), heartbeats(heartbeats) {}
        std::string nickname;
        std::string identity;
        std::string ipAddress;
        std::string port;
        std::deque<bool> greets;
        std::deque<bool> heartbeats;
        size_t sendGreetCounter = 0;
        size_t sendHeartbeatCounter = 0;
    };

    TTNetworkInterface mNetworkInterface;
    std::shared_ptr<TTContactsHandlerMock> mContactsHandler;
    std::shared_ptr<TTChatHandlerMock> mChatHandler;
    std::shared_ptr<TTNeighborsStubMock> mNeighborsStub;
    std::deque<std::string> mNeighbors;
    std::unique_ptr<TTContactsHandlerEntry> mHostEntry;
    std::map<std::string, NeighborEntry> mNeighborEntries;
    size_t mNeighborIdentityCounter;
};

TEST_F(TTBroadcasterDiscoveryTest, HappyPathReceiveGreetRequestNewNeighbor) {
    const TTGreetRequest request("John", "5e5fe55f", std::string{"192.168.1.74:"} + mNetworkInterface.getPort());
    SetNeighborCall(request.nickname, request.identity, request.ipAddressAndPort);
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    EXPECT_TRUE(broadcaster->handleGreet(request));
    EXPECT_FALSE(broadcaster->isStopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->isStopped());
}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathReceiveGreetRequestExistingNeighbor) {
    const TTGreetRequest request("John", "5e5fe55f", std::string{"192.168.1.74:"} + mNetworkInterface.getPort());
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
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    EXPECT_TRUE(broadcaster->handleGreet(request));
    EXPECT_FALSE(broadcaster->isStopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->isStopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathReceiveGreetRequestNewNeighborNoNickname) {
    const TTGreetRequest request("", "5e5fe55f", std::string{"192.168.1.74:"} + mNetworkInterface.getPort());
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    EXPECT_FALSE(broadcaster->handleGreet(request));
    EXPECT_FALSE(broadcaster->isStopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->isStopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathReceiveGreetRequestNewNeighborNoIdentity) {
    const TTGreetRequest request("John", "", std::string{"192.168.1.74:"} + mNetworkInterface.getPort());
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    EXPECT_FALSE(broadcaster->handleGreet(request));
    EXPECT_FALSE(broadcaster->isStopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->isStopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathReceiveGreetRequestNewNeighborNoIpAddressAndPort) {
    const TTGreetRequest request("John", "5e5fe55f", "");
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    EXPECT_FALSE(broadcaster->handleGreet(request));
    EXPECT_FALSE(broadcaster->isStopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->isStopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathReceiveGreetRequestNewNeighborContactsHandlerFailedToCreate) {
    const TTGreetRequest request("John", "5e5fe55f", std::string{"192.168.1.74:"} + mNetworkInterface.getPort());
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
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    EXPECT_FALSE(broadcaster->handleGreet(request));
    EXPECT_FALSE(broadcaster->isStopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->isStopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathReceiveGreetRequestNewNeighborContactsHandlerFailedToGet) {
    const TTGreetRequest request("John", "5e5fe55f", std::string{"192.168.1.74:"} + mNetworkInterface.getPort());
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
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    EXPECT_FALSE(broadcaster->handleGreet(request));
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->isStopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathReceiveGreetRequestNewNeighborChatHandlerFailedToCreate) {
    const TTGreetRequest request("John", "5e5fe55f", std::string{"192.168.1.74:"} + mNetworkInterface.getPort());
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
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    EXPECT_FALSE(broadcaster->handleGreet(request));
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->isStopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathReceiveGreetRequestExistingNeighborContactsHandlerFailedToActivate) {
    const TTGreetRequest request("John", "5e5fe55f", std::string{"192.168.1.74:"} + mNetworkInterface.getPort());
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
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    EXPECT_FALSE(broadcaster->handleGreet(request));
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->isStopped());
}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathReceiveHeartbeatRequestExistingNeighbor) {
    const TTHeartbeatRequest request("5e5fe55f");
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
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    EXPECT_TRUE(broadcaster->handleHeartbeat(request));
    EXPECT_FALSE(broadcaster->isStopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->isStopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathReceiveHeartbeatRequestNonExistingNeighbor) {
    const TTHeartbeatRequest request("5e5fe55f");
    EXPECT_CALL(*mContactsHandler, get(request.identity))
        .Times(1)
        .WillOnce(Return(std::nullopt));
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    EXPECT_FALSE(broadcaster->handleHeartbeat(request));
    EXPECT_FALSE(broadcaster->isStopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->isStopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathReceiveHeartbeatRequestExistingNeighborContactsHandlerFailedToActivate) {
    const TTHeartbeatRequest request("5e5fe55f");
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
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    EXPECT_FALSE(broadcaster->handleHeartbeat(request));
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->isStopped());
}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathGetNickname) {
    SetHostEntryAndCall("Gabrielle", "6e6e6e6", "192.168.1.74");
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    EXPECT_EQ(broadcaster->getNickname(), mHostEntry->nickname);
    EXPECT_FALSE(broadcaster->isStopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->isStopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathGetNickname) {
    EXPECT_CALL(*mContactsHandler, get(Matcher<size_t>(size_t(0))))
        .Times(1)
        .WillOnce(Return(std::nullopt));
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    EXPECT_EQ(broadcaster->getNickname(), "");
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->isStopped());
}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathGetIdentity) {
    SetHostEntryAndCall("Gabrielle", "6e6e6e6", "192.168.1.74");
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    EXPECT_EQ(broadcaster->getIdentity(), mHostEntry->identity);
    EXPECT_FALSE(broadcaster->isStopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->isStopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathGetIdentity) {
    EXPECT_CALL(*mContactsHandler, get(Matcher<size_t>(size_t(0))))
        .Times(1)
        .WillOnce(Return(std::nullopt));
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    EXPECT_EQ(broadcaster->getIdentity(), "");
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->isStopped());
}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathGetIpAddressAndPort) {
    SetHostEntryAndCall("Gabrielle", "6e6e6e6", "192.168.1.74");
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    EXPECT_EQ(broadcaster->getIpAddressAndPort(), mHostEntry->ipAddressAndPort);
    EXPECT_FALSE(broadcaster->isStopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->isStopped());
}

TEST_F(TTBroadcasterDiscoveryTest, UnhappyPathGetIpAddressAndPort) {
    EXPECT_CALL(*mContactsHandler, get(Matcher<size_t>(size_t(0))))
        .Times(1)
        .WillOnce(Return(std::nullopt));
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    EXPECT_EQ(broadcaster->getIpAddressAndPort(), "");
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->isStopped());
}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathStaticNeighborsResolvedThenEachIsActive) {
    SetHostEntryAndCall("Gabrielle", "6e6e6e6", "192.168.1.74");
    SetNeighborEntryAndCall("Oak", "14eeffe", "122.124.0.9", {true}, {true, true});
    SetNeighborEntryAndCall("Johny", "123456", "168.0.55.1", {true}, {true, true});
    SetNeighborEntryAndCall("Camille", "ddddddd", "145.111.55.8", {true}, {true, true});
    SetNeighborEntryAndCall("Steve", "9999999", "157.88.64.7", {true}, {true, true});
    SetNeighborsGreetCalls();
    SetNeighborsHeartbeatCalls();
    EXPECT_CALL(*mContactsHandler, activate(_))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    // Create broadcaster
    for (const auto &[ipAddressAndPort, entry] : mNeighborEntries) {
        mNeighbors.push_back(entry.ipAddress);
    }
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    // Start async consumer
    std::thread loop(std::bind(&TTBroadcasterDiscovery::run, broadcaster.get()));
    // Expect consumer to react
    std::this_thread::sleep_for(std::chrono::milliseconds{8000});
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    loop.join();
    for (const auto &[ipAddressAndPort, stats] : mNeighborEntries) {
        EXPECT_EQ(stats.sendGreetCounter, 1);
        EXPECT_GE(stats.sendHeartbeatCounter, 1);
    }
}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathStaticNeighborsResolvedThenEachIsInactive) {
    SetHostEntryAndCall("Gabrielle", "6e6e6e6", "192.168.1.74");
    SetNeighborEntryAndCall("Oak", "14eeffe", "122.124.0.9", {true}, {false, false});
    SetNeighborEntryAndCall("Johny", "123456", "168.0.55.1", {true}, {false, false});
    SetNeighborEntryAndCall("Camille", "ddddddd", "145.111.55.8", {true}, {false, false});
    SetNeighborEntryAndCall("Steve", "9999999", "157.88.64.7", {true}, {false, false});
    SetNeighborsGreetCalls();
    SetNeighborsHeartbeatCalls();
    EXPECT_CALL(*mContactsHandler, deactivate(_))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    // Create broadcaster
    for (const auto &[ipAddressAndPort, entry] : mNeighborEntries) {
        mNeighbors.push_back(entry.ipAddress);
    }
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    // Start async consumer
    std::thread loop(std::bind(&TTBroadcasterDiscovery::run, broadcaster.get()));
    // Expect consumer to react
    std::this_thread::sleep_for(std::chrono::milliseconds{8000});
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    loop.join();
    for (const auto &[ipAddressAndPort, stats] : mNeighborEntries) {
        EXPECT_EQ(stats.sendGreetCounter, 1);
        EXPECT_GE(stats.sendHeartbeatCounter, 1);
    }
}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathStaticNeighborsResolvedThenEachIsFlaky) {
    SetHostEntryAndCall("Gabrielle", "6e6e6e6", "192.168.1.74");
    SetNeighborEntryAndCall("Oak", "14eeffe", "122.124.0.9", {true}, {false, true, false});
    SetNeighborEntryAndCall("Johny", "123456", "168.0.55.1", {true}, {false, false, true});
    SetNeighborEntryAndCall("Camille", "ddddddd", "145.111.55.8", {true}, {true, false, true});
    SetNeighborEntryAndCall("Steve", "9999999", "157.88.64.7", {true}, {true, true, false});
    SetNeighborsGreetCalls();
    SetNeighborsHeartbeatCalls();
    EXPECT_CALL(*mContactsHandler, activate(_))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mContactsHandler, deactivate(_))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    // Create broadcaster
    for (const auto &[ipAddressAndPort, entry] : mNeighborEntries) {
        mNeighbors.push_back(entry.ipAddress);
    }
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    // Start async consumer
    std::thread loop(std::bind(&TTBroadcasterDiscovery::run, broadcaster.get()));
    // Expect consumer to react
    std::this_thread::sleep_for(std::chrono::milliseconds{13000});
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    loop.join();
    for (const auto &[ipAddressAndPort, stats] : mNeighborEntries) {
        EXPECT_EQ(stats.sendGreetCounter, 1);
        EXPECT_GE(stats.sendHeartbeatCounter, 2);
    }
}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathStaticNeighborsUnresolvedStubCreationFailed) {
    EXPECT_CALL(*mNeighborsStub, createDiscoveryStub(_))
        .Times(AtLeast(1))
        .WillRepeatedly([&](){return std::unique_ptr<TTNeighborsDiscoveryStubMock>(nullptr);});
    // Create broadcaster
    mNeighbors.push_back("122.124.0.9");
    mNeighbors.push_back("168.0.55.1");
    mNeighbors.push_back("145.111.55.8");
    mNeighbors.push_back("157.88.64.7");
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    // Start async consumer
    std::thread loop(std::bind(&TTBroadcasterDiscovery::run, broadcaster.get()));
    // Expect consumer to react
    std::this_thread::sleep_for(std::chrono::milliseconds{5000});
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    loop.join();
    for (const auto &[ipAddressAndPort, stats] : mNeighborEntries) {
        EXPECT_EQ(stats.sendGreetCounter, 0);
        EXPECT_GE(stats.sendHeartbeatCounter, 0);
    }
}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathStaticNeighborsUnresolvedGreetSendFailed) {
    const std::deque<bool> greets = {false, false, false};
    SetNeighborEntry("Oak", "14eeffe", "122.124.0.9", greets, {});
    SetNeighborEntry("Johny", "123456", "168.0.55.1", greets, {});
    SetHostEntryAndCall("Gabrielle", "6e6e6e6", "192.168.1.74");
    SetNeighborsGreetCalls();
    for (const auto &[ipAddressAndPort, entry] : mNeighborEntries) {
        InSequence __;
        for (const auto &i : greets) {
            SetNeighborCreateDiscoveryStub(ipAddressAndPort);
        }
    }
    // Create broadcaster
    for (const auto &[ipAddressAndPort, entry] : mNeighborEntries) {
        mNeighbors.push_back(entry.ipAddress);
    }
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    // Start async consumer
    std::thread loop(std::bind(&TTBroadcasterDiscovery::run, broadcaster.get()));
    // Expect consumer to react
    std::this_thread::sleep_for(std::chrono::milliseconds{5000});
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    loop.join();
    for (const auto &[ipAddressAndPort, stats] : mNeighborEntries) {
        EXPECT_EQ(stats.sendGreetCounter, 3);
        EXPECT_GE(stats.sendHeartbeatCounter, 0);
    }
}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathStaticNeighborIsResolvedThenContactsHandlerActivateFailed) {
    SetHostEntryAndCall("Gabrielle", "6e6e6e6", "192.168.1.74");
    SetNeighborEntryAndCall("Oak", "14eeffe", "122.124.0.9", {true}, {true, true, true});
    SetNeighborsGreetCalls();
    SetNeighborsHeartbeatCalls();
    EXPECT_CALL(*mContactsHandler, activate(_))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));
    // Create broadcaster
    for (const auto &[ipAddressAndPort, entry] : mNeighborEntries) {
        mNeighbors.push_back(entry.ipAddress);
    }
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    // Start async consumer
    std::thread loop(std::bind(&TTBroadcasterDiscovery::run, broadcaster.get()));
    // Expect consumer to react
    std::this_thread::sleep_for(std::chrono::milliseconds{8000});
    EXPECT_TRUE(broadcaster->isStopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    loop.join();
    for (const auto &[ipAddressAndPort, stats] : mNeighborEntries) {
        EXPECT_EQ(stats.sendGreetCounter, 1);
        EXPECT_GE(stats.sendHeartbeatCounter, 1);
    }
}

TEST_F(TTBroadcasterDiscoveryTest, HappyPathStaticNeighborIsResolvedThenContactsHandlerInactivateFailed) {
    SetHostEntryAndCall("Gabrielle", "6e6e6e6", "192.168.1.74");
    SetNeighborEntryAndCall("Oak", "14eeffe", "122.124.0.9", {true}, {false, false, false});
    SetNeighborsGreetCalls();
    SetNeighborsHeartbeatCalls();
    EXPECT_CALL(*mContactsHandler, deactivate(_))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));
    // Create broadcaster
    for (const auto &[ipAddressAndPort, entry] : mNeighborEntries) {
        mNeighbors.push_back(entry.ipAddress);
    }
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mNetworkInterface, mNeighbors);
    // Start async consumer
    std::thread loop(std::bind(&TTBroadcasterDiscovery::run, broadcaster.get()));
    // Expect consumer to react
    std::this_thread::sleep_for(std::chrono::milliseconds{8000});
    EXPECT_TRUE(broadcaster->isStopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    loop.join();
    for (const auto &[ipAddressAndPort, stats] : mNeighborEntries) {
        EXPECT_EQ(stats.sendGreetCounter, 1);
        EXPECT_GE(stats.sendHeartbeatCounter, 1);
    }
}
