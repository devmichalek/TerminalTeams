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
using ::testing::ThrowsMessage;
using ::testing::HasSubstr;

class TTEngineTest : public Test {
protected:
    TTEngineTest() {
    }
    ~TTEngineTest() {
    }
    // Called after constructor, before each test
    virtual void SetUp() override {
        // Nothing to be done
    }
    // Called before destructor, after each test
    virtual void TearDown() override {
        mEngineSettings.reset();
        mAbstractFactory.reset();
        mContactsHandler.reset();
        mChatHandler.reset();
        mTextBoxHandler.reset();
        mNeighborsStub.reset();
        mBroadcasterChat.reset();
        mBroadcasterDiscovery.reset();
        mNeighborsServiceChat.reset();
        mNeighborsServiceDiscovery.reset();
        mServer.reset();
        mNetworkInterface.reset();
        mEngine.reset();
    }

    void PrepareEngineDependencies(bool contactsHandlerStatus = true,
        bool chatHandlerStatus = true,
        bool textBoxHandlerStatus = true,
        bool neighborsStubStatus = true,
        bool broadcasterChatStatus = true,
        bool broadcasterDiscoveryStatus = true,
        bool serviceChatStatus = true,
        bool serviceDiscoveryStatus = true,
        bool serverStatus = true) {
        mEngineSettings = std::make_unique<NiceMock<TTEngineSettingsMock>>();
        mAbstractFactory = std::make_unique<TTAbstractFactoryMock>();
        mContactsHandler = std::make_unique<NiceMock<TTContactsHandlerMock>>();
        mChatHandler = std::make_unique<NiceMock<TTChatHandlerMock>>();
        mTextBoxHandler = std::make_unique<NiceMock<TTTextBoxHandlerMock>>();
        mNeighborsStub = std::make_unique<TTNeighborsStubMock>();
        mBroadcasterChat = std::make_unique<NiceMock<TTBroadcasterChatMock>>();
        mBroadcasterDiscovery = std::make_unique<NiceMock<TTBroadcasterDiscoveryMock>>();
        mNeighborsServiceChat = std::make_unique<TTNeighborsServiceChatMock>();
        mNeighborsServiceDiscovery = std::make_unique<TTNeighborsServiceDiscoveryMock>();
        mServer = std::make_unique<NiceMock<TTServerMock>>();
        mNetworkInterface = std::make_unique<TTNetworkInterface>("eno1", "192.168.1.5", "44");
        // Settings on calls
        ON_CALL(*mEngineSettings, getNetworkInterface()).WillByDefault(ReturnRef(*mNetworkInterface));
        ON_CALL(*mEngineSettings, getNickname()).WillByDefault(ReturnRef(mHostNickname));
        ON_CALL(*mEngineSettings, getIdentity()).WillByDefault(ReturnRef(mHostIdentity));
        ON_CALL(*mEngineSettings, getNeighbors()).WillByDefault(ReturnRef(mNeighbors));
        // Getter
        EXPECT_CALL(*mEngineSettings, getAbstractFactory())
            .Times(1)
            .WillOnce(ReturnRef(*mAbstractFactory));
        // Creation of the handlers
        if (contactsHandlerStatus) {
            EXPECT_CALL(*mAbstractFactory, createContactsHandler())
                .WillOnce([&](){ return std::move(mContactsHandler); });
        } else {
            EXPECT_CALL(*mAbstractFactory, createContactsHandler())
                .WillOnce([&](){ return nullptr; });
        }
        if (chatHandlerStatus) {
            EXPECT_CALL(*mAbstractFactory, createChatHandler())
                .WillOnce([&](){ return std::move(mChatHandler); });
        } else {
            EXPECT_CALL(*mAbstractFactory, createChatHandler())
                .WillOnce([&](){ return nullptr; });
        }
        if (textBoxHandlerStatus) {
            EXPECT_CALL(*mAbstractFactory, createTextBoxHandler(_, _))
                .WillOnce([&](auto callbackMessageSent, auto callbackContactsSwitch) {
                    mCallbackMessageSent = callbackMessageSent;
                    mCallbackContactsSwitch = callbackContactsSwitch;
                    return std::move(mTextBoxHandler);
                });
        } else {
            EXPECT_CALL(*mAbstractFactory, createTextBoxHandler(_, _))
                .WillOnce([&](){ return nullptr; });
        }
        if (!contactsHandlerStatus || !chatHandlerStatus || !textBoxHandlerStatus) {
            return;
        }
        // Creation of the first contact
        EXPECT_CALL(*mContactsHandler, create(mHostNickname, mHostIdentity, mNetworkInterface->getIpAddressAndPort()))
            .Times(1)
            .WillOnce(Return(true));
        EXPECT_CALL(*mContactsHandler, select(0))
            .Times(1)
            .WillOnce(Return(true));
        EXPECT_CALL(*mChatHandler, create(0))
            .Times(1)
            .WillOnce(Return(true));
        EXPECT_CALL(*mChatHandler, select(0))
            .Times(1)
            .WillOnce(Return(true));
        // Creation of the neighbors stub
        if (neighborsStubStatus) {
            EXPECT_CALL(*mAbstractFactory, createNeighborsStub())
                .WillOnce([&](){ return std::move(mNeighborsStub); });
        } else {
            EXPECT_CALL(*mAbstractFactory, createNeighborsStub())
                .WillOnce([&](){ return nullptr; });
            return;
        }
        // Creation of the broadcasters
        if (broadcasterChatStatus) {
            EXPECT_CALL(*mAbstractFactory, createBroadcasterChat(_, _, _, _))
                .WillOnce([&](){ return std::move(mBroadcasterChat); });
        } else {
            EXPECT_CALL(*mAbstractFactory, createBroadcasterChat(_, _, _, _))
                .WillOnce([&](){ return nullptr; });
        }
        if (broadcasterDiscoveryStatus) {
            EXPECT_CALL(*mAbstractFactory, createBroadcasterDiscovery(_, _, _, _, _))
                .WillOnce([&](){ return std::move(mBroadcasterDiscovery); });
        } else {
            EXPECT_CALL(*mAbstractFactory, createBroadcasterDiscovery(_, _, _, _, _))
                .WillOnce([&](){ return nullptr; });
        }
        if (!broadcasterChatStatus || !broadcasterDiscoveryStatus) {
            return;
        }
        // Creation of the services
        if (serviceChatStatus) {
            EXPECT_CALL(*mAbstractFactory, createNeighborsServiceChat(_))
                .WillOnce([&](){ return std::move(mNeighborsServiceChat); });
        } else {
            EXPECT_CALL(*mAbstractFactory, createNeighborsServiceChat(_))
                .WillOnce([&](){ return nullptr; });
        }
        if (serviceDiscoveryStatus) {
            EXPECT_CALL(*mAbstractFactory, createNeighborsServiceDiscovery(_))
                .WillOnce([&](){ return std::move(mNeighborsServiceDiscovery); });
        } else {
            EXPECT_CALL(*mAbstractFactory, createNeighborsServiceDiscovery(_))
                .WillOnce([&](){ return nullptr; });
        }
        if (!serviceChatStatus || !serviceDiscoveryStatus) {
            return;
        }
        // Creation of the server
        if (serverStatus) {
            EXPECT_CALL(*mAbstractFactory, createServer(_, _, _))
                .WillOnce([&](){ return std::move(mServer); });
        } else {
            EXPECT_CALL(*mAbstractFactory, createServer(_, _, _))
                .WillOnce([&](){ return nullptr; });
            return;
        }
        
        // Stop flags
        mContactsStopFlag.store(false);
        mChatStopFlag.store(false);
        mTextBoxStopFlag.store(false);
        mBroadcasterChatStopFlag.store(false);
        mBroadcasterDiscoveryStopFlag.store(false);
        mServerStopFlag.store(false);
        ON_CALL(*mContactsHandler, stopped()).WillByDefault([&](){ return mContactsStopFlag.load(); });
        ON_CALL(*mChatHandler, stopped()).WillByDefault([&](){ return mChatStopFlag.load(); });
        ON_CALL(*mTextBoxHandler, stopped()).WillByDefault([&](){ return mTextBoxStopFlag.load(); });
        ON_CALL(*mBroadcasterChat, stopped()).WillByDefault([&](){ return mBroadcasterChatStopFlag.load(); });
        ON_CALL(*mBroadcasterDiscovery, stopped()).WillByDefault([&](){ return mBroadcasterDiscoveryStopFlag.load(); });
        ON_CALL(*mServer, stopped()).WillByDefault([&](){ return mServerStopFlag.load(); });
        ON_CALL(*mContactsHandler, stop()).WillByDefault([&](){ mContactsStopFlag.store(true); });
        ON_CALL(*mChatHandler, stop()).WillByDefault([&](){ mChatStopFlag.store(true); });
        ON_CALL(*mTextBoxHandler, stop()).WillByDefault([&](){ mTextBoxStopFlag.store(true); });
        ON_CALL(*mBroadcasterChat, stop()).WillByDefault([&](){ mBroadcasterChatStopFlag.store(true); mBroadcasterChatCondition.notify_one(); });
        ON_CALL(*mBroadcasterDiscovery, stop()).WillByDefault([&](){ mBroadcasterDiscoveryStopFlag.store(true); mBroadcasterDiscoveryCondition.notify_one(); });
        ON_CALL(*mServer, stop()).WillByDefault([&](){ mServerStopFlag.store(true); mServerCondition.notify_one(); });
        // Running threads main methods
        EXPECT_CALL(*mBroadcasterChat, run())
            .Times(1)
            .WillOnce([&]() {
                std::unique_lock<std::mutex> lock(mBroadcasterChatMutex);
                mBroadcasterChatCondition.wait(lock, [this]() { return mBroadcasterChatStopFlag.load(); });
            });
        EXPECT_CALL(*mBroadcasterDiscovery, run())
            .Times(1)
            .WillOnce([&]() {
                std::unique_lock<std::mutex> lock(mBroadcasterDiscoveryMutex);
                mBroadcasterDiscoveryCondition.wait(lock, [this]() { return mBroadcasterDiscoveryStopFlag.load(); });
            });
        EXPECT_CALL(*mServer, run())
            .Times(1)
            .WillOnce([&]() {
                std::unique_lock<std::mutex> lock(mServerMutex);
                mServerCondition.wait(lock, [this]() { return mServerStopFlag.load(); });
            });
        
    }

    void CreateEngine() {
        mEngine = std::make_unique<TTEngine>(*mEngineSettings);
    }

    std::unique_ptr<NiceMock<TTEngineSettingsMock>> mEngineSettings;
    std::unique_ptr<TTAbstractFactoryMock> mAbstractFactory;
    std::unique_ptr<NiceMock<TTContactsHandlerMock>> mContactsHandler;
    std::unique_ptr<NiceMock<TTChatHandlerMock>> mChatHandler;
    std::unique_ptr<NiceMock<TTTextBoxHandlerMock>> mTextBoxHandler;
    std::unique_ptr<TTNeighborsStubMock> mNeighborsStub;
    std::unique_ptr<NiceMock<TTBroadcasterChatMock>> mBroadcasterChat;
    std::unique_ptr<NiceMock<TTBroadcasterDiscoveryMock>> mBroadcasterDiscovery;
    std::unique_ptr<TTNeighborsServiceChatMock> mNeighborsServiceChat;
    std::unique_ptr<TTNeighborsServiceDiscoveryMock> mNeighborsServiceDiscovery;
    std::unique_ptr<NiceMock<TTServerMock>> mServer;
    std::unique_ptr<TTNetworkInterface> mNetworkInterface;
    inline static const std::string mHostNickname = "adriqun";
    inline static const std::string mHostIdentity = "00DEAD00BEAF00";
    inline static const std::deque<std::string> mNeighbors = {"192.168.1.9:1", "192.168.1.10:2", "192.168.1.11:3"};
    std::atomic<bool> mContactsStopFlag;
    std::atomic<bool> mChatStopFlag;
    std::atomic<bool> mTextBoxStopFlag;
    std::atomic<bool> mBroadcasterChatStopFlag;
    std::atomic<bool> mBroadcasterDiscoveryStopFlag;
    std::atomic<bool> mServerStopFlag;
    std::mutex mBroadcasterChatMutex;
    std::mutex mBroadcasterDiscoveryMutex;
    std::mutex mServerMutex;
    std::condition_variable mBroadcasterChatCondition;
    std::condition_variable mBroadcasterDiscoveryCondition;
    std::condition_variable mServerCondition;
    TTTextBoxCallbackMessageSent mCallbackMessageSent;
    TTTextBoxCallbackContactSwitch mCallbackContactsSwitch;
    std::unique_ptr<TTEngine> mEngine;
};

TEST_F(TTEngineTest, HappyPathEngineStop) {
    PrepareEngineDependencies();
    CreateEngine();
    EXPECT_FALSE(mEngine->stopped());
    std::thread loop(std::bind(&TTEngine::run, mEngine.get()));
    std::this_thread::sleep_for(std::chrono::milliseconds{150});
    EXPECT_FALSE(mEngine->stopped());
    mEngine->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    EXPECT_TRUE(mEngine->stopped());
    loop.join();
}

TEST_F(TTEngineTest, HappyPathContactsHandlerStop) {
    PrepareEngineDependencies();
    CreateEngine();
    EXPECT_FALSE(mEngine->stopped());
    std::thread loop(std::bind(&TTEngine::run, mEngine.get()));
    std::this_thread::sleep_for(std::chrono::milliseconds{150});
    EXPECT_FALSE(mEngine->stopped());
    mContactsStopFlag.store(true);
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    EXPECT_TRUE(mEngine->stopped());
    loop.join();
}

TEST_F(TTEngineTest, HappyPathChatHandlerStop) {
    PrepareEngineDependencies();
    CreateEngine();
    EXPECT_FALSE(mEngine->stopped());
    std::thread loop(std::bind(&TTEngine::run, mEngine.get()));
    std::this_thread::sleep_for(std::chrono::milliseconds{150});
    EXPECT_FALSE(mEngine->stopped());
    mChatStopFlag.store(true);
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    EXPECT_TRUE(mEngine->stopped());
    loop.join();
}

TEST_F(TTEngineTest, HappyPathTextBoxHandlerStop) {
    PrepareEngineDependencies();
    CreateEngine();
    EXPECT_FALSE(mEngine->stopped());
    std::thread loop(std::bind(&TTEngine::run, mEngine.get()));
    std::this_thread::sleep_for(std::chrono::milliseconds{150});
    EXPECT_FALSE(mEngine->stopped());
    mTextBoxStopFlag.store(true);
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    EXPECT_TRUE(mEngine->stopped());
    loop.join();
}

TEST_F(TTEngineTest, HappyPathBroadcasterChatHandlerStop) {
    PrepareEngineDependencies();
    CreateEngine();
    EXPECT_FALSE(mEngine->stopped());
    std::thread loop(std::bind(&TTEngine::run, mEngine.get()));
    std::this_thread::sleep_for(std::chrono::milliseconds{150});
    EXPECT_FALSE(mEngine->stopped());
    mBroadcasterChatStopFlag.store(true);
    mBroadcasterChatCondition.notify_one();
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    EXPECT_TRUE(mEngine->stopped());
    loop.join();
}

TEST_F(TTEngineTest, HappyPathBroadcasterDiscoveryHandlerStop) {
    PrepareEngineDependencies();
    CreateEngine();
    EXPECT_FALSE(mEngine->stopped());
    std::thread loop(std::bind(&TTEngine::run, mEngine.get()));
    std::this_thread::sleep_for(std::chrono::milliseconds{150});
    EXPECT_FALSE(mEngine->stopped());
    mBroadcasterDiscoveryStopFlag.store(true);
    mBroadcasterDiscoveryCondition.notify_one();
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    EXPECT_TRUE(mEngine->stopped());
    loop.join();
}

TEST_F(TTEngineTest, HappyPathServerHandlerStop) {
    PrepareEngineDependencies();
    CreateEngine();
    EXPECT_FALSE(mEngine->stopped());
    std::thread loop(std::bind(&TTEngine::run, mEngine.get()));
    std::this_thread::sleep_for(std::chrono::milliseconds{150});
    EXPECT_FALSE(mEngine->stopped());
    mServerStopFlag.store(true);
    mServerCondition.notify_one();
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    EXPECT_TRUE(mEngine->stopped());
    loop.join();
}

TEST_F(TTEngineTest, UnhappyPathHandlersCreationFailed) {
    EXPECT_THAT([&]() {PrepareEngineDependencies(false, true, true); CreateEngine();},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngine: Failed to create handlers!")));
    EXPECT_THAT([&]() {PrepareEngineDependencies(true, false, true); CreateEngine();},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngine: Failed to create handlers!")));
    EXPECT_THAT([&]() {PrepareEngineDependencies(true, true, false); CreateEngine();},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngine: Failed to create handlers!")));
    EXPECT_THAT([&]() {PrepareEngineDependencies(false, false, true); CreateEngine();},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngine: Failed to create handlers!")));
    EXPECT_THAT([&]() {PrepareEngineDependencies(true, false, false); CreateEngine();},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngine: Failed to create handlers!")));
    EXPECT_THAT([&]() {PrepareEngineDependencies(false, true, false); CreateEngine();},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngine: Failed to create handlers!")));
    EXPECT_THAT([&]() {PrepareEngineDependencies(false, false, false); CreateEngine();},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngine: Failed to create handlers!")));
}

TEST_F(TTEngineTest, UnhappyPathNeighborsStubCreationFailed) {
    EXPECT_THAT([&]() {PrepareEngineDependencies(true, true, true, false); CreateEngine();},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngine: Failed to create neighbors stub!")));
}

TEST_F(TTEngineTest, UnhappyPathBroadcastersCreationFailed) {
    EXPECT_THAT([&]() {PrepareEngineDependencies(true, true, true, true, false, true); CreateEngine();},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngine: Failed to create broadcasters!")));
    EXPECT_THAT([&]() {PrepareEngineDependencies(true, true, true, true, true, false); CreateEngine();},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngine: Failed to create broadcasters!")));
    EXPECT_THAT([&]() {PrepareEngineDependencies(true, true, true, true, false, false); CreateEngine();},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngine: Failed to create broadcasters!")));
}

TEST_F(TTEngineTest, UnhappyPathServicesCreationFailed) {
    EXPECT_THAT([&]() {PrepareEngineDependencies(true, true, true, true, true, true, false, true); CreateEngine();},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngine: Failed to create services!")));
    EXPECT_THAT([&]() {PrepareEngineDependencies(true, true, true, true, true, true, true, false); CreateEngine();},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngine: Failed to create services!")));
    EXPECT_THAT([&]() {PrepareEngineDependencies(true, true, true, true, true, true, false, false); CreateEngine();},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngine: Failed to create services!")));
}

TEST_F(TTEngineTest, UnhappyPathServerCreationFailed) {
    EXPECT_THAT([&]() {PrepareEngineDependencies(true, true, true, true, true, true, true, true, false); CreateEngine();},
        ThrowsMessage<std::runtime_error>(HasSubstr("TTEngine: Failed to create gRPC server!")));
}

TEST_F(TTEngineTest, HappyPathMailbox) {
    PrepareEngineDependencies();
    const std::string message = ";) 2024";
    EXPECT_CALL(*mBroadcasterChat, handleSend(message))
        .Times(1)
        .WillOnce(Return(true));
    CreateEngine();
    EXPECT_FALSE(mEngine->stopped());
    std::thread loop(std::bind(&TTEngine::run, mEngine.get()));
    std::this_thread::sleep_for(std::chrono::milliseconds{150});
    EXPECT_FALSE(mEngine->stopped());
    mCallbackMessageSent(message);
    EXPECT_FALSE(mEngine->stopped());
    mEngine->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    EXPECT_TRUE(mEngine->stopped());
    loop.join();
}

TEST_F(TTEngineTest, UnhappyPathMailboxBroadcasterChatFailedToHandle) {
    PrepareEngineDependencies();
    const std::string message = ";( 2024";
    EXPECT_CALL(*mBroadcasterChat, handleSend(message))
        .Times(1)
        .WillOnce(Return(false));
    CreateEngine();
    EXPECT_FALSE(mEngine->stopped());
    std::thread loop(std::bind(&TTEngine::run, mEngine.get()));
    std::this_thread::sleep_for(std::chrono::milliseconds{150});
    EXPECT_FALSE(mEngine->stopped());
    mCallbackMessageSent(message);
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    EXPECT_TRUE(mEngine->stopped());
    loop.join();
}

TEST_F(TTEngineTest, HappyPathSwitcher) {
    PrepareEngineDependencies();
    const size_t message = 2;
    EXPECT_CALL(*mContactsHandler, size())
        .Times(1)
        .WillOnce(Return(3));
    EXPECT_CALL(*mContactsHandler, select(message))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mChatHandler, select(message))
        .Times(1)
        .WillOnce(Return(true));
    CreateEngine();
    EXPECT_FALSE(mEngine->stopped());
    std::thread loop(std::bind(&TTEngine::run, mEngine.get()));
    std::this_thread::sleep_for(std::chrono::milliseconds{150});
    EXPECT_FALSE(mEngine->stopped());
    mCallbackContactsSwitch(message);
    EXPECT_FALSE(mEngine->stopped());
    mEngine->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    EXPECT_TRUE(mEngine->stopped());
    loop.join();
}

TEST_F(TTEngineTest, UnhappyPathSwitcherIdIsTooLarge) {
    PrepareEngineDependencies();
    const size_t message = 2;
    EXPECT_CALL(*mContactsHandler, size())
        .Times(1)
        .WillOnce(Return(1));
    CreateEngine();
    EXPECT_FALSE(mEngine->stopped());
    std::thread loop(std::bind(&TTEngine::run, mEngine.get()));
    std::this_thread::sleep_for(std::chrono::milliseconds{150});
    EXPECT_FALSE(mEngine->stopped());
    mCallbackContactsSwitch(message);
    EXPECT_FALSE(mEngine->stopped());
    mEngine->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    EXPECT_TRUE(mEngine->stopped());
    loop.join();
}

TEST_F(TTEngineTest, UnhappyPathSwitcherContactsHandlerSelectFailed) {
    PrepareEngineDependencies();
    const size_t message = 2;
    EXPECT_CALL(*mContactsHandler, size())
        .Times(1)
        .WillOnce(Return(3));
    EXPECT_CALL(*mContactsHandler, select(message))
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_CALL(*mChatHandler, select(message))
        .Times(1)
        .WillOnce(Return(true));
    CreateEngine();
    EXPECT_FALSE(mEngine->stopped());
    std::thread loop(std::bind(&TTEngine::run, mEngine.get()));
    std::this_thread::sleep_for(std::chrono::milliseconds{150});
    EXPECT_FALSE(mEngine->stopped());
    mCallbackContactsSwitch(message);
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    EXPECT_TRUE(mEngine->stopped());
    loop.join();
}

TEST_F(TTEngineTest, UnhappyPathSwitcherChatHandlerSelectFailed) {
    PrepareEngineDependencies();
    const size_t message = 2;
    EXPECT_CALL(*mContactsHandler, size())
        .Times(1)
        .WillOnce(Return(3));
    EXPECT_CALL(*mContactsHandler, select(message))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mChatHandler, select(message))
        .Times(1)
        .WillOnce(Return(false));
    CreateEngine();
    EXPECT_FALSE(mEngine->stopped());
    std::thread loop(std::bind(&TTEngine::run, mEngine.get()));
    std::this_thread::sleep_for(std::chrono::milliseconds{150});
    EXPECT_FALSE(mEngine->stopped());
    mCallbackContactsSwitch(message);
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    EXPECT_TRUE(mEngine->stopped());
    loop.join();
}

TEST_F(TTEngineTest, UnhappyPathSwitcherContactsHandlerAndChatHandlerSelectFailed) {
    PrepareEngineDependencies();
    const size_t message = 2;
    EXPECT_CALL(*mContactsHandler, size())
        .Times(1)
        .WillOnce(Return(3));
    EXPECT_CALL(*mContactsHandler, select(message))
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_CALL(*mChatHandler, select(message))
        .Times(1)
        .WillOnce(Return(false));
    CreateEngine();
    EXPECT_FALSE(mEngine->stopped());
    std::thread loop(std::bind(&TTEngine::run, mEngine.get()));
    std::this_thread::sleep_for(std::chrono::milliseconds{150});
    EXPECT_FALSE(mEngine->stopped());
    mCallbackContactsSwitch(message);
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    EXPECT_TRUE(mEngine->stopped());
    loop.join();
}
