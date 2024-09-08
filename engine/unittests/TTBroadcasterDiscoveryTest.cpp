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
        mContactsHandler = std::make_shared<TTContactsHandlerMock>();
        mChatHandler = std::make_shared<TTChatHandlerMock>();
        mNeighborsStub = std::make_shared<TTNeighborsStubMock>();
    }
    ~TTBroadcasterDiscoveryTest() {
    }
    // Called after constructor, before each test
    virtual void SetUp() override {
        // Nothing to be done
    }
    // Called before destructor, after each test
    virtual void TearDown() override {
        // Nothing to be done
    }
    TTNetworkInterface mInterface;
    std::shared_ptr<TTContactsHandlerMock> mContactsHandler;
    std::shared_ptr<TTChatHandlerMock> mChatHandler;
    std::shared_ptr<TTNeighborsStubMock> mNeighborsStub;
};

TEST_F(TTBroadcasterDiscoveryTest, HappyPathReceiveGreetRequest) {
    TTGreetRequest request;
    request.nickname="John";
    request.identity="5e5fe55f";
    request.ipAddressAndPort="192.168.1.74:58";
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
            .WillOnce(Return(true));
        EXPECT_CALL(*mNeighborsStub, createDiscoveryStub(request.ipAddressAndPort))
            .Times(1)
            .WillOnce(Return(ByMove(nullptr)));
    }
    std::deque<std::string> neighbors;
    auto broadcaster = std::make_unique<TTBroadcasterDiscovery>(*mContactsHandler, *mChatHandler, *mNeighborsStub, mInterface, neighbors);
    EXPECT_TRUE(broadcaster->handleGreet(request));
    EXPECT_FALSE(broadcaster->stopped());
    broadcaster->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_TRUE(broadcaster->stopped());
}
