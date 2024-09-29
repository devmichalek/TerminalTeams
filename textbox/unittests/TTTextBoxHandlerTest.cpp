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
        mReceivedContactSelections.clear();
        mExpectedContactSelections.clear();
    }

    void RestartApplication() {
        mHandler = std::make_unique<TTTextBoxHandler>(*mSettingsMock,
            std::bind(&TTTextBoxHandlerTest::MessageReceiver, this, _1),
            std::bind(&TTTextBoxHandlerTest::ContactsSelectionReceiver, this, _1));
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

    void ContactsSelectionReceiver(size_t id) {
        mReceivedContactSelections.emplace_back(id);
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
        return TTTextBoxMessage{TTTextBoxStatus::MESSAGE, static_cast<unsigned int>(data.size()), data.c_str()};
    }

    TTTextBoxMessage createContactsSelectionMessage(size_t id) {
        mExpectedContactSelections.push_back(id);
        return TTTextBoxMessage{TTTextBoxStatus::CONTACTS_SELECT, sizeof(id), reinterpret_cast<char*>(&id)};
    }

    std::shared_ptr<TTTextBoxSettingsMock> mSettingsMock;
    std::shared_ptr<TTUtilsNamedPipeMock> mNamedPipeMock;
    std::unique_ptr<TTTextBoxHandler> mHandler;
    std::vector<std::string> mReceivedMessages;
    std::vector<std::string> mExpectedMessages;
    std::vector<size_t> mReceivedContactSelections;
    std::vector<size_t> mExpectedContactSelections;
};

ACTION_P(SetArgPointerInReceiveMessage, rhs) {
    std::memcpy(arg0, &rhs, sizeof(rhs));
}

TEST_F(TTTextBoxHandlerTest, FailedToOpenNamedPipe) {
    EXPECT_CALL(*mNamedPipeMock, open)
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_THROW(RestartApplication(), std::runtime_error);
}

TEST_F(TTTextBoxHandlerTest, FailedToRunNamedPipeIsNotAlive) {
    EXPECT_CALL(*mNamedPipeMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(false));
    RestartApplication();
    VerifyApplicationTimeout(std::chrono::milliseconds{100});
}

TEST_F(TTTextBoxHandlerTest, FailedToReceiveMessage) {
    EXPECT_CALL(*mNamedPipeMock, open)
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
    EXPECT_CALL(*mNamedPipeMock, open)
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
    EXPECT_CALL(*mNamedPipeMock, open)
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
    EXPECT_CALL(*mNamedPipeMock, open)
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
    EXPECT_CALL(*mNamedPipeMock, open)
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

TEST_F(TTTextBoxHandlerTest, SuccessReceivedContactsSelect) {
    EXPECT_CALL(*mNamedPipeMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    const auto heartbeatMessage = createHeartbeatMessage();
    const std::vector<TTTextBoxMessage> messages1 = {
        createContactsSelectionMessage(5),
        createContactsSelectionMessage(1),
        createContactsSelectionMessage(0)
    };
    const std::vector<TTTextBoxMessage> messages2 = {
        createContactsSelectionMessage(1005),
        createContactsSelectionMessage(1),
        createContactsSelectionMessage(100),
        createContactsSelectionMessage(25),
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
    EXPECT_EQ(mExpectedContactSelections.size(), mReceivedContactSelections.size());
    for (size_t i = 0; i < mExpectedContactSelections.size(); ++i) {
        EXPECT_EQ(mExpectedContactSelections[i], mReceivedContactSelections[i]);
    }
}

TEST_F(TTTextBoxHandlerTest, SuccessReceivedMessage) {
    EXPECT_CALL(*mNamedPipeMock, open)
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
    EXPECT_CALL(*mNamedPipeMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    const auto heartbeatMessage = createHeartbeatMessage();
    const std::vector<TTTextBoxMessage> messages1 = {
        createMessage("Hello"),
        createMessage(" World!"),
        createContactsSelectionMessage(3)
    };
    const std::vector<TTTextBoxMessage> messages2 = {
        createMessage("Are"),
        createMessage("you ok?"),
        createContactsSelectionMessage(3)
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
    EXPECT_EQ(mExpectedContactSelections.size(), mReceivedContactSelections.size());
    for (size_t i = 0; i < mExpectedContactSelections.size(); ++i) {
        EXPECT_EQ(mExpectedContactSelections[i], mReceivedContactSelections[i]);
    }
}
