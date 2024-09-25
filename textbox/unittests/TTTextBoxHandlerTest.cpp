#include "TTTextBoxHandler.hpp"
#include "TTTextBoxSettingsMock.hpp"
#include "TTUtilsNamedPipeMock.hpp"
#include "TTTextBoxMessage.hpp"
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
        mReceivedMessages.clear();
        mExpectedMessages.clear();
        mReceivedContactSwitches.clear();
        mExpectedContactSwitches.clear();
    }

    void RestartApplication() {
        mHandler = std::make_unique<TTTextBoxHandler>(*mSettingsMock,
            std::bind(&TTTextBoxHandlerTest::MessageReceiver, this, _1),
            std::bind(&TTTextBoxHandlerTest::ContactsSwitchReceiver, this, _1));
        EXPECT_FALSE(mHandler->isStopped());
    }

    void VerifyApplicationTimeout(std::chrono::milliseconds timeout) {
        std::this_thread::sleep_for(timeout);
        EXPECT_TRUE(mHandler->isStopped());
        mHandler.reset();
    }

    void MessageReceiver(const std::string& message) {
        mReceivedMessages.emplace_back(message);
    }

    void ContactsSwitchReceiver(size_t id) {
        mReceivedContactSwitches.emplace_back(id);
    }

    TTTextBoxMessage createUndefinedMessage() {
        return TTTextBoxMessage{TTTextBoxStatus::UNDEFINED, 0, nullptr};
    }

    TTTextBoxMessage createUnknownMessage() {
        TTTextBoxMessage result(TTTextBoxStatus::UNDEFINED, 0, nullptr);
        result.status = static_cast<TTTextBoxStatus>(0xFFFF);
        return result;
    }

    TTTextBoxMessage createHeartbeatMessage() {
        return TTTextBoxMessage{TTTextBoxStatus::HEARTBEAT, 0, nullptr};
    }

    TTTextBoxMessage createGoodbyeMessage() {
        return TTTextBoxMessage{TTTextBoxStatus::GOODBYE, 0, nullptr};
    }

    TTTextBoxMessage createMessage(const std::string& data) {
        mExpectedMessages.push_back(data);
        return TTTextBoxMessage{TTTextBoxStatus::MESSAGE, data.size(), data.c_str()};
    }

    TTTextBoxMessage createContactsSwitchMessage(size_t id) {
        mExpectedContactSwitches.push_back(id);
        return TTTextBoxMessage{TTTextBoxStatus::CONTACTS_SWITCH, sizeof(id), reinterpret_cast<char*>(&id)};
    }

    std::shared_ptr<TTTextBoxSettingsMock> mSettingsMock;
    std::shared_ptr<TTUtilsNamedPipeMock> mNamedPipeMock;
    std::unique_ptr<TTTextBoxHandler> mHandler;
    std::vector<std::string> mReceivedMessages;
    std::vector<std::string> mExpectedMessages;
    std::vector<size_t> mReceivedContactSwitches;
    std::vector<size_t> mExpectedContactSwitches;
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

TEST_F(TTTextBoxHandlerTest, FailedBecauseOfUndefinedMessage) {
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    const auto unknownMessage = createUndefinedMessage();
    EXPECT_CALL(*mNamedPipeMock, receive)
        .WillRepeatedly(DoAll(SetArgPointerInReceiveMessage(unknownMessage), Return(true)));
    RestartApplication();
    VerifyApplicationTimeout(std::chrono::milliseconds{100});
}

TEST_F(TTTextBoxHandlerTest, FailedBecauseOfUnknownMessage) {
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    const auto unknownMessage = createUnknownMessage();
    EXPECT_CALL(*mNamedPipeMock, receive)
        .WillRepeatedly(DoAll(SetArgPointerInReceiveMessage(unknownMessage), Return(true)));
    RestartApplication();
    VerifyApplicationTimeout(std::chrono::milliseconds{100});
}

TEST_F(TTTextBoxHandlerTest, SuccessReceivedHeartbeats) {
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    const auto heartbeatMessage = createHeartbeatMessage();
    EXPECT_CALL(*mNamedPipeMock, receive)
        .WillRepeatedly(DoAll(SetArgPointerInReceiveMessage(heartbeatMessage), Return(true)));
    RestartApplication();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    mHandler->stop();
    VerifyApplicationTimeout(std::chrono::milliseconds{100});
}

TEST_F(TTTextBoxHandlerTest, SuccessReceivedGoodbye) {
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    const auto goodbyeMessage = createGoodbyeMessage();
    EXPECT_CALL(*mNamedPipeMock, receive)
        .Times(1)
        .WillOnce(DoAll(SetArgPointerInReceiveMessage(goodbyeMessage), Return(true)));
    RestartApplication();
    VerifyApplicationTimeout(std::chrono::milliseconds{50});
}

TEST_F(TTTextBoxHandlerTest, SuccessReceivedContactsSwitch) {
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    const auto heartbeatMessage = createHeartbeatMessage();
    const std::vector<TTTextBoxMessage> messages1 = {
        createContactsSwitchMessage(5),
        createContactsSwitchMessage(1),
        createContactsSwitchMessage(0)
    };
    const std::vector<TTTextBoxMessage> messages2 = {
        createContactsSwitchMessage(1005),
        createContactsSwitchMessage(1),
        createContactsSwitchMessage(100),
        createContactsSwitchMessage(25),
    };
    {
        InSequence _;
        EXPECT_CALL(*mNamedPipeMock, receive)
            .Times(1)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(heartbeatMessage), Return(true)));
        for (const auto& msg : messages1) {
            EXPECT_CALL(*mNamedPipeMock, receive)
                .Times(1)
                .WillOnce(DoAll(SetArgPointerInReceiveMessage(msg), Return(true)));
        }
        EXPECT_CALL(*mNamedPipeMock, receive)
            .Times(1)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(heartbeatMessage), Return(true)));
        for (const auto& msg : messages2) {
            EXPECT_CALL(*mNamedPipeMock, receive)
                .Times(1)
                .WillOnce(DoAll(SetArgPointerInReceiveMessage(msg), Return(true)));
        }
        EXPECT_CALL(*mNamedPipeMock, receive)
            .Times(AtLeast(1))
            .WillRepeatedly(DoAll(SetArgPointerInReceiveMessage(heartbeatMessage), Return(true)));
    }
    RestartApplication();
    std::this_thread::sleep_for(std::chrono::milliseconds{1000});
    mHandler->stop();
    VerifyApplicationTimeout(std::chrono::milliseconds{100});
    // Verify
    EXPECT_EQ(mExpectedContactSwitches.size(), mReceivedContactSwitches.size());
    for (size_t i = 0; i < mExpectedContactSwitches.size(); ++i) {
        EXPECT_EQ(mExpectedContactSwitches[i], mReceivedContactSwitches[i]);
    }
}

TEST_F(TTTextBoxHandlerTest, SuccessReceivedMessage) {
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    const auto heartbeatMessage = createHeartbeatMessage();
    const std::vector<TTTextBoxMessage> messages1 = {
        createMessage("Hello"),
        createMessage(" John"),
        createMessage("How you doing?")
    };
    const std::vector<TTTextBoxMessage> messages2 = {
        createMessage("Good "),
        createMessage("thank you"),
        createMessage(" for asking"),
        createMessage("What about you?")
    };
    {
        InSequence _;
        EXPECT_CALL(*mNamedPipeMock, receive)
            .Times(1)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(heartbeatMessage), Return(true)));
        for (const auto& msg : messages1) {
            EXPECT_CALL(*mNamedPipeMock, receive)
                .Times(1)
                .WillOnce(DoAll(SetArgPointerInReceiveMessage(msg), Return(true)));
        }
        EXPECT_CALL(*mNamedPipeMock, receive)
            .Times(1)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(heartbeatMessage), Return(true)));
        for (const auto& msg : messages2) {
            EXPECT_CALL(*mNamedPipeMock, receive)
                .Times(1)
                .WillOnce(DoAll(SetArgPointerInReceiveMessage(msg), Return(true)));
        }
        EXPECT_CALL(*mNamedPipeMock, receive)
            .Times(AtLeast(1))
            .WillRepeatedly(DoAll(SetArgPointerInReceiveMessage(heartbeatMessage), Return(true)));
    }
    RestartApplication();
    std::this_thread::sleep_for(std::chrono::milliseconds{1000});
    mHandler->stop();
    VerifyApplicationTimeout(std::chrono::milliseconds{100});
    // Verify
    EXPECT_EQ(mExpectedMessages.size(), mReceivedMessages.size());
    for (size_t i = 0; i < mExpectedMessages.size(); ++i) {
        EXPECT_EQ(mExpectedMessages[i], mReceivedMessages[i]);
    }
}

TEST_F(TTTextBoxHandlerTest, SuccessReceivedMixOfMessages) {
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    const auto heartbeatMessage = createHeartbeatMessage();
    const std::vector<TTTextBoxMessage> messages1 = {
        createMessage("Hello"),
        createMessage(" World!"),
        createContactsSwitchMessage(3)
    };
    const std::vector<TTTextBoxMessage> messages2 = {
        createMessage("Are"),
        createMessage("you ok?"),
        createContactsSwitchMessage(3)
    };
    const auto goodbyeMessage = createGoodbyeMessage();
    {
        InSequence _;
        EXPECT_CALL(*mNamedPipeMock, receive)
            .Times(1)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(heartbeatMessage), Return(true)));
        for (const auto& msg : messages1) {
            EXPECT_CALL(*mNamedPipeMock, receive)
                .Times(1)
                .WillOnce(DoAll(SetArgPointerInReceiveMessage(msg), Return(true)));
        }
        EXPECT_CALL(*mNamedPipeMock, receive)
            .Times(1)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(heartbeatMessage), Return(true)));
        for (const auto& msg : messages2) {
            EXPECT_CALL(*mNamedPipeMock, receive)
                .Times(1)
                .WillOnce(DoAll(SetArgPointerInReceiveMessage(msg), Return(true)));
        }
        EXPECT_CALL(*mNamedPipeMock, receive)
            .Times(1)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(goodbyeMessage), Return(true)));
    }
    RestartApplication();
    std::this_thread::sleep_for(std::chrono::milliseconds{1000});
    VerifyApplicationTimeout(std::chrono::milliseconds{100});
    // Verify
    EXPECT_EQ(mExpectedMessages.size(), mReceivedMessages.size());
    for (size_t i = 0; i < mExpectedMessages.size(); ++i) {
        EXPECT_EQ(mExpectedMessages[i], mReceivedMessages[i]);
    }
    EXPECT_EQ(mExpectedContactSwitches.size(), mReceivedContactSwitches.size());
    for (size_t i = 0; i < mExpectedContactSwitches.size(); ++i) {
        EXPECT_EQ(mExpectedContactSwitches[i], mReceivedContactSwitches[i]);
    }
}
