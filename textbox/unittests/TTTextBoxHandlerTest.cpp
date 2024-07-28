#include "TTTextBoxHandler.hpp"
#include "TTTextBoxSettingsMock.hpp"
#include "TTUtilsNamedPipeMock.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstring>

using ::testing::Test;
using ::testing::Return;
using ::testing::InSequence;
using ::testing::_;
using ::testing::DoAll;
using ::testing::AtLeast;
using namespace std::placeholders;

class TTTextBoxHandlerTest : public Test {
protected:
    TTTextBoxHandlerTest() {
        mSettingsMock = std::make_shared<TTTextBoxSettingsMock>();
        mNamedPipeMock = std::make_shared<TTUtilsNamedPipeMock>();
    }
    ~TTTextBoxHandlerTest() {

    }
    // Called after constructor, before each test
    virtual void SetUp() override {
        EXPECT_CALL(*mSettingsMock, getNamedPipe)
            .Times(1)
            .WillOnce(Return(mNamedPipeMock));
    }
    // Called before destructor, after each test
    virtual void TearDown() override {

    }

    void RestartApplication() {
        mHandler = std::make_unique<TTTextBoxHandler>(*mSettingsMock,
            std::bind(&TTTextBoxHandlerTest::MessageReceiver, this, _1),
            std::bind(&TTTextBoxHandlerTest::ContactsSwitchReceiver, this, _1));
        EXPECT_FALSE(mHandler->stopped());
    }

    void VerifyApplicationTimeout(std::chrono::milliseconds timeout) {
        std::this_thread::sleep_for(timeout);
        EXPECT_TRUE(mHandler->stopped());
        mHandler.reset();
    }

    void MessageReceiver(const std::string& message) {
        mReceivedMessages.emplace_back(message);
    }

    void ContactsSwitchReceiver(size_t id) {
        mReceivedContactSwitches.emplace_back(id);
    }

    std::shared_ptr<TTTextBoxSettingsMock> mSettingsMock;
    std::shared_ptr<TTUtilsNamedPipeMock> mNamedPipeMock;
    std::unique_ptr<TTTextBoxHandler> mHandler;
    std::vector<std::string> mReceivedMessages;
    std::vector<size_t> mReceivedContactSwitches;
};

ACTION_P(SetArgPointerInReceiveMessage, rhs) {
    std::memcpy(arg0, &rhs, sizeof(rhs));
}

TEST_F(TTTextBoxHandlerTest, FailedToCreateNamedPipe) {
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_THROW(RestartApplication(), std::runtime_error);
}

TEST_F(TTTextBoxHandlerTest, FailedToRunNamedPipeIsNotAlive) {
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(false));
    RestartApplication();
    VerifyApplicationTimeout(std::chrono::milliseconds{100});
}

TEST_F(TTTextBoxHandlerTest, FailedToReceiveMessage) {
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, receive)
        .Times(AtLeast(2))
        .WillRepeatedly(Return(false));
    RestartApplication();
    VerifyApplicationTimeout(std::chrono::milliseconds{2000});
}

// TEST_F(TTTextBoxHandlerTest, FailedToReceiveMessage) {
//     EXPECT_CALL(*mNamedPipeMock, create)
//         .Times(1)
//         .WillOnce(Return(true));
//     EXPECT_CALL(*mNamedPipeMock, alive)
//         .Times(1)
//         .WillOnce(Return(true));
//     {
//         InSequence _;
//         EXPECT_CALL(*mSharedMemMock, receive)
//             .WillRepeatedly(DoAll(SetArgPointerInReceiveMessage(heartbeat), Return(false)));
//     }
//     RestartApplication();
//     VerifyApplicationTimeout(std::chrono::milliseconds{100});
// }

// TEST_F(TTTextBoxHandlerTest, FailedBecauseOfUndefinedMessage) {
    
// }

// TEST_F(TTTextBoxHandlerTest, FailedBecauseOfUnknownMessage) {
    
// }

// TEST_F(TTTextBoxHandlerTest, SuccessReceivedHeartbeats) {

// }

// TEST_F(TTTextBoxHandlerTest, SuccessReceivedContactsSwitch) {

// }

// TEST_F(TTTextBoxHandlerTest, SuccessReceivedMessage) {

// }

// TEST_F(TTTextBoxHandlerTest, SuccessReceivedMixOfMessages) {

// }
