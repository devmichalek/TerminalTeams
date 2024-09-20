#include "TTEngine.hpp"
#include "TTEngineSettingsMock.hpp"
#include "TTAbstractFactoryMock.hpp"
#include "TTContactsHandlerMock.hpp"
#include "TTChatHandlerMock.hpp"
#include "TTTextBoxHandlerMock.hpp"
#include "TTNeighborsStubMock.hpp"
#include "TTBroadcasterChatMock.hpp"
#include "TTBroadcasterDiscoveryMock.hpp"
#include "TTNeighborsServiceChatMock.hpp"
#include "TTNeighborsServiceDiscoveryMock.hpp"
#include "TTServerMock.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::Test;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::ByMove;
using ::testing::AtLeast;
using ::testing::NiceMock;
using ::testing::_;

class TTEngineTest : public Test {
protected:
    TTEngineTest() {
    }
    ~TTEngineTest() {
    }
    // Called after constructor, before each test
    virtual void SetUp() override {
        mEngineSettings = std::make_unique<TTEngineSettingsMock>();
        mAbstractFactory = std::make_unique<TTAbstractFactoryMock>();
        mContactsHandler = std::make_unique<NiceMock<TTContactsHandlerMock>>();
        mChatHandler = std::make_unique<NiceMock<TTChatHandlerMock>>();
        mTextBoxHandler = std::make_unique<NiceMock<TTTextBoxHandlerMock>>();
        mNeighborsStub = std::make_unique<TTNeighborsStubMock>();
        mBroadcasterChat = std::make_unique<NiceMock<TTBroadcasterChatMock>>();
        mBroadcasterDiscovery = std::make_unique<NiceMock<TTBroadcasterDiscoveryMock>>();
        mNeighborsServiceChat = std::make_unique<TTNeighborsServiceChatMock>();
        mNeighborsServiceDiscovery = std::make_unique<TTNeighborsServiceDiscoveryMock>();
        mServer = std::make_unique<TTServerMock>();
        mNetworkInterface = std::make_unique<TTNetworkInterface>("eno1", "192.168.1.5", "44");
        EXPECT_CALL(*mEngineSettings, getAbstractFactory())
            .Times(1)
            .WillOnce(ReturnRef(*mAbstractFactory));
        EXPECT_CALL(*mEngineSettings, getNetworkInterface())
            .Times(AtLeast(1))
            .WillRepeatedly(ReturnRef(*mNetworkInterface));
        EXPECT_CALL(*mEngineSettings, getNickname())
            .Times(1)
            .WillOnce(ReturnRef(mHostNickname));
        EXPECT_CALL(*mEngineSettings, getIdentity())
            .Times(1)
            .WillOnce(ReturnRef(mHostIdentity));
        EXPECT_CALL(*mEngineSettings, getNeighbors())
            .Times(1)
            .WillOnce(ReturnRef(mNeighbors));
        EXPECT_CALL(*mAbstractFactory, createContactsHandler())
            .Times(1)
            .WillOnce([&](){ return std::move(mContactsHandler); });
        EXPECT_CALL(*mAbstractFactory, createChatHandler())
            .Times(1)
            .WillOnce([&](){ return std::move(mChatHandler); });
        EXPECT_CALL(*mAbstractFactory, createTextBoxHandler(_, _))
            .Times(1)
            .WillOnce([&](){ return std::move(mTextBoxHandler); });
        EXPECT_CALL(*mAbstractFactory, createNeighborsStub())
            .Times(1)
            .WillOnce([&](){ return std::move(mNeighborsStub); });
        EXPECT_CALL(*mAbstractFactory, createBroadcasterChat(_, _, _, _))
            .Times(1)
            .WillOnce([&](){ return std::move(mBroadcasterChat); });
        EXPECT_CALL(*mAbstractFactory, createBroadcasterDiscovery(_, _, _, _, _))
            .Times(1)
            .WillOnce([&](){ return std::move(mBroadcasterDiscovery); });
        EXPECT_CALL(*mAbstractFactory, createNeighborsServiceChat(_))
            .Times(1)
            .WillOnce([&](){ return std::move(mNeighborsServiceChat); });
        EXPECT_CALL(*mAbstractFactory, createNeighborsServiceDiscovery(_))
            .Times(1)
            .WillOnce([&](){ return std::move(mNeighborsServiceDiscovery); });
        EXPECT_CALL(*mAbstractFactory, createServer(_, _, _))
            .Times(1)
            .WillOnce([&](){ return std::move(mServer); });
        mContactsStopFlag.store(false);
        mChatStopFlag.store(false);
        mTextBoxStopFlag.store(false);
        mBroadcasterChatStopFlag.store(false);
        mroadcasterDiscoveryStopFlag.store(false);
        ON_CALL(*mContactsHandler, stopped()).WillByDefault([&](){ return mContactsStopFlag.load(); });
        ON_CALL(*mChatHandler, stopped()).WillByDefault([&](){ return mChatStopFlag.load(); });
        ON_CALL(*mTextBoxHandler, stopped()).WillByDefault([&](){ return mTextBoxStopFlag.load(); });
        ON_CALL(*mBroadcasterChat, stopped()).WillByDefault([&](){ return mBroadcasterChatStopFlag.load(); });
        ON_CALL(*mBroadcasterDiscovery, stopped()).WillByDefault([&](){ return mroadcasterDiscoveryStopFlag.load(); });
        mEngine = std::make_unique<TTEngine>(*mEngineSettings);
    }
    // Called before destructor, after each test
    virtual void TearDown() override {
        mEngineSettings.reset();
        mAbstractFactory.reset();
        mEngine.reset();
    }
    std::unique_ptr<TTEngineSettingsMock> mEngineSettings;
    std::unique_ptr<TTAbstractFactoryMock> mAbstractFactory;
    std::unique_ptr<NiceMock<TTContactsHandlerMock>> mContactsHandler;
    std::unique_ptr<NiceMock<TTChatHandlerMock>> mChatHandler;
    std::unique_ptr<NiceMock<TTTextBoxHandlerMock>> mTextBoxHandler;
    std::unique_ptr<TTNeighborsStubMock> mNeighborsStub;
    std::unique_ptr<NiceMock<TTBroadcasterChatMock>> mBroadcasterChat;
    std::unique_ptr<NiceMock<TTBroadcasterDiscoveryMock>> mBroadcasterDiscovery;
    std::unique_ptr<TTNeighborsServiceChatMock> mNeighborsServiceChat;
    std::unique_ptr<TTNeighborsServiceDiscoveryMock> mNeighborsServiceDiscovery;
    std::unique_ptr<TTServerMock> mServer;
    std::unique_ptr<TTNetworkInterface> mNetworkInterface;
    inline static const std::string mHostNickname = "adriqun";
    inline static const std::string mHostIdentity = "00DEAD00BEAF00";
    inline static const std::deque<std::string> mNeighbors = {"192.168.1.9:1", "192.168.1.10:2", "192.168.1.11:3"};
    std::atomic<bool> mContactsStopFlag;
    std::atomic<bool> mChatStopFlag;
    std::atomic<bool> mTextBoxStopFlag;
    std::atomic<bool> mBroadcasterChatStopFlag;
    std::atomic<bool> mroadcasterDiscoveryStopFlag;
    std::unique_ptr<TTEngine> mEngine;
};

TEST_F(TTEngineTest, HappyPath) {
    EXPECT_FALSE(mEngine->stopped());
    mEngine->run();
    EXPECT_TRUE(mEngine->stopped());
}
