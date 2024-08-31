#include "TTBroadcasterChat.hpp"
#include "TTContactsHandlerMock.hpp"
#include "TTChatHandlerMock.hpp"
#include "TTNeighborsStubMock.hpp"
#include "TTNeighborsChatStubMock.hpp"
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

class TTBroadcasterChatTest : public Test {
protected:
    TTBroadcasterChatTest() : mInterface("eno1", "192.168.1.1", "1777") {
        mContactsHandler = std::make_shared<TTContactsHandlerMock>();
        mChatHandler = std::make_shared<TTChatHandlerMock>();
        mNeighborsStub = std::make_shared<TTNeighborsStubMock>();
    }
    ~TTBroadcasterChatTest() {
    }
    // Called after constructor, before each test
    virtual void SetUp() override {
        mBroadcaster = std::make_unique<TTBroadcasterChat>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface);
    }
    // Called before destructor, after each test
    virtual void TearDown() override {
        mBroadcaster.reset();
    }
    TTNetworkInterface mInterface;
    std::shared_ptr<TTContactsHandlerMock> mContactsHandler;
    std::shared_ptr<TTChatHandlerMock> mChatHandler;
    std::shared_ptr<TTNeighborsStubMock> mNeighborsStub;
    std::unique_ptr<TTBroadcasterChat> mBroadcaster;
};

TEST_F(TTBroadcasterChatTest, HappyPathReceiveTellRequest) {
    TTTellRequest request;
    request.identity = "312382290f4f71e7fb7f00449fb529fce3b8ec95";
    request.message = "Hello world!";
    std::optional<size_t> id = 1;
    EXPECT_CALL(*mContactsHandler, get(request.identity))
        .Times(1)
        .WillOnce(Return(id));
    EXPECT_CALL(*mContactsHandler, receive(id.value()))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mChatHandler, receive(id.value(), request.message, _))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(mBroadcaster->handleReceive(request));
}

TEST_F(TTBroadcasterChatTest, UnhappyPathReceiveTellRequestNoIdentity) {
    TTTellRequest request;
    request.identity = "312382290f4f71e7fb7f00449fb529fce3b8ec95";
    request.message = "Hello world!";
    EXPECT_CALL(*mContactsHandler, get(request.identity))
        .Times(1)
        .WillOnce(Return(std::nullopt));
    EXPECT_FALSE(mBroadcaster->handleReceive(request));
}

TEST_F(TTBroadcasterChatTest, UnhappyPathReceiveTellRequestContactsHandlerFailed) {
    TTTellRequest request;
    request.identity = "312382290f4f71e7fb7f00449fb529fce3b8ec95";
    request.message = "Hello world!";
    std::optional<size_t> id = 1;
    EXPECT_CALL(*mContactsHandler, get(request.identity))
        .Times(1)
        .WillOnce(Return(id));
    EXPECT_CALL(*mContactsHandler, receive(id.value()))
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_FALSE(mBroadcaster->handleReceive(request));
}

TEST_F(TTBroadcasterChatTest, UnhappyPathReceiveTellRequestChatHandlerFailed) {
    TTTellRequest request;
    request.identity = "312382290f4f71e7fb7f00449fb529fce3b8ec95";
    request.message = "Hello world!";
    std::optional<size_t> id = 1;
    EXPECT_CALL(*mContactsHandler, get(request.identity))
        .Times(1)
        .WillOnce(Return(id));
    EXPECT_CALL(*mContactsHandler, receive(id.value()))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mChatHandler, receive(id.value(), request.message, _))
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_FALSE(mBroadcaster->handleReceive(request));
}

TEST_F(TTBroadcasterChatTest, HappyPathReceiveNarrateRequest) {
    TTNarrateRequest request;
    request.identity = "312382290f4f71e7fb7f00449fb529fce3b8ec95";
    request.messages = { "a", "b", "c" };
    std::optional<size_t> id = 1;
    EXPECT_CALL(*mContactsHandler, get(request.identity))
        .Times(1)
        .WillOnce(Return(id));
    EXPECT_CALL(*mContactsHandler, receive(id.value()))
        .Times(1)
        .WillOnce(Return(true));
    for (const auto& message : request.messages) {
        EXPECT_CALL(*mChatHandler, receive(id.value(), message, _))
            .Times(1)
            .WillOnce(Return(true));
    }
    EXPECT_TRUE(mBroadcaster->handleReceive(request));
}

TEST_F(TTBroadcasterChatTest, UnhappyPathReceiveNarrateRequestNoIdentity) {
    TTNarrateRequest request;
    request.identity = "312382290f4f71e7fb7f00449fb529fce3b8ec95";
    request.messages = { "a", "b", "c" };
    EXPECT_CALL(*mContactsHandler, get(request.identity))
        .Times(1)
        .WillOnce(Return(std::nullopt));
    EXPECT_FALSE(mBroadcaster->handleReceive(request));
}

TEST_F(TTBroadcasterChatTest, UnhappyPathReceiveNarrateRequestContactsHandlerFailed) {
    TTNarrateRequest request;
    request.identity = "312382290f4f71e7fb7f00449fb529fce3b8ec95";
    request.messages = { "a", "b", "c" };
    std::optional<size_t> id = 1;
    EXPECT_CALL(*mContactsHandler, get(request.identity))
        .Times(1)
        .WillOnce(Return(id));
    EXPECT_CALL(*mContactsHandler, receive(id.value()))
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_FALSE(mBroadcaster->handleReceive(request));
}

TEST_F(TTBroadcasterChatTest, UnhappyPathReceiveNarrateRequestChatHandlerFailed) {
    TTNarrateRequest request;
    request.identity = "312382290f4f71e7fb7f00449fb529fce3b8ec95";
    request.messages = { "a", "b", "c" };
    std::optional<size_t> id = 1;
    EXPECT_CALL(*mContactsHandler, get(request.identity))
        .Times(1)
        .WillOnce(Return(id));
    EXPECT_CALL(*mContactsHandler, receive(id.value()))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mChatHandler, receive(id.value(), request.messages.front(), _))
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_FALSE(mBroadcaster->handleReceive(request));
}

TEST_F(TTBroadcasterChatTest, HappyPathGetIdentity) {
    TTContactsHandlerEntry entry("nickname", "312382290f4f71e7fb7f00449fb529fce3b8ec95", "192.168.1.88:875");
    EXPECT_CALL(*mContactsHandler, get(Matcher<size_t>(size_t(0))))
        .Times(1)
        .WillOnce(Return(entry));
    EXPECT_EQ(mBroadcaster->getIdentity(), entry.identity);
}

TEST_F(TTBroadcasterChatTest, UnhappyPathGetIdentity) {
    EXPECT_CALL(*mContactsHandler, get(Matcher<size_t>(size_t(0))))
        .Times(1)
        .WillOnce(Return(std::nullopt));
    EXPECT_TRUE(mBroadcaster->getIdentity().empty());
}

TEST_F(TTBroadcasterChatTest, UnhappyPathSendContactsHandlerCurrentFailed) {
    EXPECT_CALL(*mContactsHandler, current())
        .Times(1)
        .WillOnce(Return(std::nullopt));
    EXPECT_FALSE(mBroadcaster->handleSend("foo"));
    EXPECT_TRUE(mBroadcaster->stopped());
}

TEST_F(TTBroadcasterChatTest, UnhappyPathSendChatHandlerCurrentFailed) {
    EXPECT_CALL(*mContactsHandler, current())
        .Times(1)
        .WillOnce(Return(1));
    EXPECT_CALL(*mChatHandler, current())
        .Times(1)
        .WillOnce(Return(std::nullopt));
    EXPECT_FALSE(mBroadcaster->handleSend("foo"));
    EXPECT_TRUE(mBroadcaster->stopped());
}

TEST_F(TTBroadcasterChatTest, UnhappyPathSendIdentityMismatch) {
    EXPECT_CALL(*mContactsHandler, current())
        .Times(1)
        .WillOnce(Return(1));
    EXPECT_CALL(*mChatHandler, current())
        .Times(1)
        .WillOnce(Return(2));
    EXPECT_FALSE(mBroadcaster->handleSend("foo"));
    EXPECT_TRUE(mBroadcaster->stopped());
}

TEST_F(TTBroadcasterChatTest, UnhappyPathSendContactsHandlerSendFailed) {
    const size_t currentId = 1;
    EXPECT_CALL(*mContactsHandler, current())
        .Times(1)
        .WillOnce(Return(currentId));
    EXPECT_CALL(*mChatHandler, current())
        .Times(1)
        .WillOnce(Return(currentId));
    EXPECT_CALL(*mContactsHandler, send(currentId))
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_FALSE(mBroadcaster->handleSend("foo"));
    EXPECT_TRUE(mBroadcaster->stopped());
}

TEST_F(TTBroadcasterChatTest, UnhappyPathSendChatHandlerSendFailed) {
    const size_t currentId = 1;
    const std::string message = "foo";
    EXPECT_CALL(*mContactsHandler, current())
        .Times(1)
        .WillOnce(Return(currentId));
    EXPECT_CALL(*mChatHandler, current())
        .Times(1)
        .WillOnce(Return(currentId));
    EXPECT_CALL(*mContactsHandler, send(currentId))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mChatHandler, send(currentId, message, _))
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_FALSE(mBroadcaster->handleSend(message));
    EXPECT_TRUE(mBroadcaster->stopped());
}

TEST_F(TTBroadcasterChatTest, UnhappyPathSendContactsHandlerGetFailed) {
    const size_t currentId = 1;
    const std::string message = "foo";
    EXPECT_CALL(*mContactsHandler, current())
        .Times(1)
        .WillOnce(Return(currentId));
    EXPECT_CALL(*mChatHandler, current())
        .Times(1)
        .WillOnce(Return(currentId));
    EXPECT_CALL(*mContactsHandler, send(currentId))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mChatHandler, send(currentId, message, _))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mContactsHandler, get(currentId))
        .Times(1)
        .WillOnce(Return(std::nullopt));
    EXPECT_FALSE(mBroadcaster->handleSend(message));
    EXPECT_TRUE(mBroadcaster->stopped());
}

TEST_F(TTBroadcasterChatTest, HappyPathSendHostMatch) {
    const size_t currentId = 1;
    const std::string message = "foo";
    const TTContactsHandlerEntry entry("host", "312382290f4f71e7fb7f00449fb529fce3b8ec95", mInterface.getIpAddressAndPort());
    EXPECT_CALL(*mContactsHandler, current())
        .Times(1)
        .WillOnce(Return(currentId));
    EXPECT_CALL(*mChatHandler, current())
        .Times(1)
        .WillOnce(Return(currentId));
    EXPECT_CALL(*mContactsHandler, send(currentId))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mChatHandler, send(currentId, message, _))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mContactsHandler, get(currentId))
        .Times(1)
        .WillOnce(Return(entry));
    EXPECT_TRUE(mBroadcaster->handleSend(message));
    EXPECT_FALSE(mBroadcaster->stopped());
}

TEST_F(TTBroadcasterChatTest, UnhappyPathSendTellImmediateStubCreationFailed) {
    // Setup
    const size_t currentId = 1;
    const std::string message = "foo";
    const std::string neighborIdentity = "f71e7fb";
    const std::string hostIdentity = "888992ef";
    TTTellRequest expectedRequest;
    expectedRequest.message = message;
    expectedRequest.identity = hostIdentity;
    const TTContactsHandlerEntry entry("neighbor", neighborIdentity, "192.168.1.80:8879");
    const TTContactsHandlerEntry hostEntry("host", hostIdentity, mInterface.getIpAddressAndPort());
    EXPECT_CALL(*mContactsHandler, current())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(currentId));
    EXPECT_CALL(*mChatHandler, current())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(currentId));
    EXPECT_CALL(*mContactsHandler, send(currentId))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mChatHandler, send(currentId, message, _))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mContactsHandler, get(currentId))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(entry));
    EXPECT_CALL(*mContactsHandler, get(Matcher<size_t>(size_t(0))))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(hostEntry));
    {
        InSequence __;
        EXPECT_CALL(*mNeighborsStub, createChatStub(entry.ipAddressAndPort))
            .Times(1)
            .WillOnce(Return(ByMove(nullptr)));
        EXPECT_CALL(*mNeighborsStub, createChatStub(entry.ipAddressAndPort))
            .Times(1)
            .WillOnce(Return(ByMove(std::make_unique<TTNeighborsChatStubMock>())));
        EXPECT_CALL(*mNeighborsStub, sendTell(_, expectedRequest))
            .Times(1)
            .WillOnce(Return(TTTellResponse{true}));
    }
    // Start async consumer
    std::thread loop(std::bind(&TTBroadcasterChat::run, mBroadcaster.get()));
    // Send message
    EXPECT_TRUE(mBroadcaster->handleSend(message));
    EXPECT_FALSE(mBroadcaster->stopped());
    // Expect consumer to react
    std::this_thread::sleep_for(std::chrono::milliseconds{1000});
    mBroadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    loop.join();
}

TEST_F(TTBroadcasterChatTest, UnhappyPathSendTellLazyStubCreationFailed) {
    // Setup
    const size_t currentId = 1;
    const std::string message = "foo";
    const std::string neighborIdentity = "f71e7fb";
    const TTContactsHandlerEntry entry("neighbor", neighborIdentity, "192.168.1.80:8879");
    EXPECT_CALL(*mContactsHandler, current())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(currentId));
    EXPECT_CALL(*mChatHandler, current())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(currentId));
    EXPECT_CALL(*mContactsHandler, send(currentId))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mChatHandler, send(currentId, message, _))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mContactsHandler, get(currentId))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(entry));
    EXPECT_CALL(*mNeighborsStub, createChatStub(entry.ipAddressAndPort))
        .Times(AtLeast(2))
        .WillRepeatedly([&](){ return std::unique_ptr<TTNeighborsChatStubMock>(nullptr); });
    // Start async consumer
    std::thread loop(std::bind(&TTBroadcasterChat::run, mBroadcaster.get()));
    // Send message
    EXPECT_TRUE(mBroadcaster->handleSend(message));
    EXPECT_FALSE(mBroadcaster->stopped());
    // Expect consumer to react
    std::this_thread::sleep_for(std::chrono::milliseconds{8000});
    mBroadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    loop.join();
}

TEST_F(TTBroadcasterChatTest, HappyPathSendTellImmediateStubCreationFailed) {
    // Setup
    const size_t currentId = 1;
    const std::string message = "foo";
    const std::string neighborIdentity = "f71e7fb";
    const std::string hostIdentity = "888992ef";
    TTTellRequest expectedRequest;
    expectedRequest.message = message;
    expectedRequest.identity = hostIdentity;
    const TTContactsHandlerEntry neighborEntry("neighbor", neighborIdentity, "192.168.1.80:8879");
    const TTContactsHandlerEntry hostEntry("host", hostIdentity, mInterface.getIpAddressAndPort());
    EXPECT_CALL(*mContactsHandler, current())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(currentId));
    EXPECT_CALL(*mChatHandler, current())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(currentId));
    EXPECT_CALL(*mContactsHandler, send(currentId))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mChatHandler, send(currentId, message, _))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mContactsHandler, get(currentId))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(neighborEntry));
    EXPECT_CALL(*mContactsHandler, get(Matcher<size_t>(size_t(0))))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(hostEntry));
    {
        InSequence __;
        EXPECT_CALL(*mNeighborsStub, createChatStub(neighborEntry.ipAddressAndPort))
            .Times(2)
            .WillRepeatedly([&](){return std::unique_ptr<TTNeighborsChatStubMock>(nullptr); });
        EXPECT_CALL(*mNeighborsStub, createChatStub(neighborEntry.ipAddressAndPort))
            .Times(1)
            .WillOnce(Return(ByMove(std::make_unique<TTNeighborsChatStubMock>())));
        EXPECT_CALL(*mNeighborsStub, sendTell(_, expectedRequest))
            .Times(1)
            .WillOnce(Return(TTTellResponse{true}));
    }
    // Start async consumer
    std::thread loop(std::bind(&TTBroadcasterChat::run, mBroadcaster.get()));
    // Send message
    EXPECT_TRUE(mBroadcaster->handleSend(message));
    EXPECT_FALSE(mBroadcaster->stopped());
    // Expect consumer to react
    std::this_thread::sleep_for(std::chrono::milliseconds{20000}); // max inactivity timeout (6s)
    mBroadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    loop.join();
}

TEST_F(TTBroadcasterChatTest, HappyPathSendNarrateImmediateStubCreationFailed) {
    // Setup
    const size_t currentId = 1;
    const std::vector<std::string> messages{
        "foo",
        "boo",
        "lazy"
    };
    const std::string neighborIdentity = "f71e7fb";
    const std::string hostIdentity = "888992ef";
    TTNarrateRequest expectedRequest;
    expectedRequest.identity = hostIdentity;
    for (const auto& message : messages) {
        expectedRequest.messages.push_back(message);
    }
    const TTContactsHandlerEntry neighborEntry("neighbor", neighborIdentity, "192.168.1.80:8879");
    const TTContactsHandlerEntry hostEntry("host", hostIdentity, mInterface.getIpAddressAndPort());
    EXPECT_CALL(*mContactsHandler, current())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(currentId));
    EXPECT_CALL(*mChatHandler, current())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(currentId));
    EXPECT_CALL(*mContactsHandler, send(currentId))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    for (const auto& message : messages) {
        EXPECT_CALL(*mChatHandler, send(currentId, message, _))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(true));
    }
    EXPECT_CALL(*mContactsHandler, get(currentId))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(neighborEntry));
    EXPECT_CALL(*mContactsHandler, get(Matcher<size_t>(size_t(0))))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(hostEntry));
    {
        InSequence __;
        EXPECT_CALL(*mNeighborsStub, createChatStub(neighborEntry.ipAddressAndPort))
            .Times(2)
            .WillRepeatedly([&](){return std::unique_ptr<TTNeighborsChatStubMock>(nullptr); });
        EXPECT_CALL(*mNeighborsStub, createChatStub(neighborEntry.ipAddressAndPort))
            .Times(1)
            .WillOnce(Return(ByMove(std::make_unique<TTNeighborsChatStubMock>())));
        EXPECT_CALL(*mNeighborsStub, sendNarrate(_, expectedRequest))
            .Times(1)
            .WillOnce(Return(TTNarrateResponse{true}));
    }
    // Start async consumer
    std::thread loop(std::bind(&TTBroadcasterChat::run, mBroadcaster.get()));
    // Send message
    for (const auto& message : messages) {
        EXPECT_TRUE(mBroadcaster->handleSend(message));
    }
    EXPECT_FALSE(mBroadcaster->stopped());
    // Expect consumer to react
    std::this_thread::sleep_for(std::chrono::milliseconds{20000});
    mBroadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    loop.join();
}
