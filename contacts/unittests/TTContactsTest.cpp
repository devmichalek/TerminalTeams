#include "TTContacts.hpp"
#include "TTContactsSettingsMock.hpp"
#include "TTUtilsSharedMemMock.hpp"
#include "TTUtilsOutputStreamMock.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <memory>

using ::testing::Test;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::InSequence;
using ::testing::_;

class TTContactsTest : public Test {
protected:
    TTContactsTest() {
        mSettingsMock = std::make_shared<TTContactsSettingsMock>();
        mSharedMemMock = std::make_shared<TTUtilsSharedMemMock>();
        mOutputStreamMock = std::make_shared<TTUtilsOutputStreamMock>();
    }
    ~TTContactsTest() {

    }
    // Called after constructor, before each test
    virtual void SetUp() override {
        EXPECT_CALL(*mSettingsMock, getTerminalWidth).Times(1);
        EXPECT_CALL(*mSettingsMock, getTerminalHeight).Times(1);
        EXPECT_CALL(*mSettingsMock, getSharedMemory)
            .Times(1)
            .WillOnce(Return(mSharedMemMock));
    }
    // Called before destructor, after each test
    virtual void TearDown() override {
        mOutputStreamMock->mOutput.clear();
    }

    void StartApplication() {
        mContacts->run();
        mApplicationCv.notify_one();
    }

    void RestartApplication(std::chrono::milliseconds timeout) {
        mApplicationTimeout = timeout;
        mContacts = std::make_unique<TTContacts>(*mSettingsMock, *mOutputStreamMock);
        EXPECT_FALSE(mContacts->isStopped());
        mApplicationThread = std::thread{&TTContactsTest::StartApplication, this};
    }

    void VerifyApplicationTimeout() {
        std::unique_lock<std::mutex> lock(mApplicationMutex);
        const bool predicate = mApplicationCv.wait_for(lock, mApplicationTimeout, [this]() {
            return mContacts->isStopped();
        });
        EXPECT_TRUE(predicate); // Check for application timeout
        EXPECT_TRUE(mContacts->isStopped());
        mApplicationThread.join();
    }

    TTContactsMessage CreateMessage(TTContactsStatus status,
                                    TTContactsState state = TTContactsState::ACTIVE,
                                    size_t identity = 0,
                                    std::string nickname = "") const {
        TTContactsMessage result;
        result.setStatus(status);
        result.setState(state);
        result.setIdentity(identity);
        result.setNickname(nickname);
        return result;
    }

    std::shared_ptr<TTContactsSettingsMock> mSettingsMock;
    std::shared_ptr<TTUtilsSharedMemMock> mSharedMemMock;
    std::shared_ptr<TTUtilsOutputStreamMock> mOutputStreamMock;
    std::unique_ptr<TTContacts> mContacts;
    std::chrono::milliseconds mApplicationTimeout;
    std::thread mApplicationThread;
    std::mutex mApplicationMutex;
    std::condition_variable mApplicationCv;
};

ACTION_P(SetArgPointerInReceiveMessage, rhs) {
    std::memcpy(arg0, &rhs, sizeof(rhs));
}

TEST_F(TTContactsTest, SharedMemoryInitFailed) {
    EXPECT_CALL(*mSharedMemMock, open)
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_THROW(RestartApplication(std::chrono::milliseconds{0}), std::runtime_error);
}

TEST_F(TTContactsTest, SimpleStopNoRunning) {
    EXPECT_CALL(*mSharedMemMock, open)
        .Times(1)
        .WillOnce(Return(true));
    RestartApplication(std::chrono::milliseconds{500});
    mContacts->stop();
    VerifyApplicationTimeout();
}

TEST_F(TTContactsTest, ThreeHeartbeatsThenFailedToReceiveMessage) {
    EXPECT_CALL(*mSharedMemMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSharedMemMock, alive)
        .WillRepeatedly(Return(true));
    TTContactsMessage heartbeat;
    heartbeat.setStatus(TTContactsStatus::HEARTBEAT);
    {
        InSequence _;
        EXPECT_CALL(*mSharedMemMock, receive)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(heartbeat), Return(true)));
        EXPECT_CALL(*mSharedMemMock, receive)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(heartbeat), Return(true)));
        EXPECT_CALL(*mSharedMemMock, receive)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(heartbeat), Return(true)));
        EXPECT_CALL(*mSharedMemMock, receive)
            .WillOnce(Return(false));
    }
    RestartApplication(std::chrono::milliseconds{500});
    VerifyApplicationTimeout();
}

TEST_F(TTContactsTest, ThreeHeartbeatsThenSharedMemoryNotAlive) {
    EXPECT_CALL(*mSharedMemMock, open)
        .Times(1)
        .WillOnce(Return(true));
    TTContactsMessage heartbeat;
    heartbeat.setStatus(TTContactsStatus::HEARTBEAT);
    {
        InSequence _;
        EXPECT_CALL(*mSharedMemMock, alive)
            .WillOnce(Return(true));
        EXPECT_CALL(*mSharedMemMock, receive)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(heartbeat), Return(true)));
        EXPECT_CALL(*mSharedMemMock, alive)
            .WillOnce(Return(true));
        EXPECT_CALL(*mSharedMemMock, receive)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(heartbeat), Return(true)));
        EXPECT_CALL(*mSharedMemMock, alive)
            .WillOnce(Return(true));
        EXPECT_CALL(*mSharedMemMock, receive)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(heartbeat), Return(true)));
        EXPECT_CALL(*mSharedMemMock, alive)
            .WillOnce(Return(false));
    }
    RestartApplication(std::chrono::milliseconds{500});
    VerifyApplicationTimeout();
}

TEST_F(TTContactsTest, ThreeHeartbeatsThenGoodbyeMessage) {
    EXPECT_CALL(*mSharedMemMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSharedMemMock, alive)
        .WillRepeatedly(Return(true));
    TTContactsMessage heartbeat;
    heartbeat.setStatus(TTContactsStatus::HEARTBEAT);
    TTContactsMessage goodbye;
    goodbye.setStatus(TTContactsStatus::GOODBYE);
    {
        InSequence _;
        EXPECT_CALL(*mSharedMemMock, receive)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(heartbeat), Return(true)));
        EXPECT_CALL(*mSharedMemMock, receive)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(heartbeat), Return(true)));
        EXPECT_CALL(*mSharedMemMock, receive)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(heartbeat), Return(true)));
        EXPECT_CALL(*mSharedMemMock, receive)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(goodbye), Return(true)));
    }
    RestartApplication(std::chrono::milliseconds{500});
    VerifyApplicationTimeout();
}

TEST_F(TTContactsTest, ThreeHeartbeatsThenUnknownMessage) {
    EXPECT_CALL(*mSharedMemMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSharedMemMock, alive)
        .WillRepeatedly(Return(true));
    TTContactsMessage heartbeat;
    heartbeat.setStatus(TTContactsStatus::HEARTBEAT);
    TTContactsMessage unknown;
    unknown.setStatus(static_cast<TTContactsStatus>(std::numeric_limits<size_t>::max()));
    {
        InSequence _;
        EXPECT_CALL(*mSharedMemMock, receive)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(heartbeat), Return(true)));
        EXPECT_CALL(*mSharedMemMock, receive)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(heartbeat), Return(true)));
        EXPECT_CALL(*mSharedMemMock, receive)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(heartbeat), Return(true)));
        EXPECT_CALL(*mSharedMemMock, receive)
            .WillOnce(DoAll(SetArgPointerInReceiveMessage(unknown), Return(true)));
    }
    RestartApplication(std::chrono::milliseconds{500});
    VerifyApplicationTimeout();
}

TEST_F(TTContactsTest, OneHeartbeatThreeNewContactsOneSelected) {
    EXPECT_CALL(*mSharedMemMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSharedMemMock, alive)
        .WillRepeatedly(Return(true));
    std::vector<TTContactsMessage> messages;
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 2, "C"));
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_ACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::GOODBYE));
    {
        InSequence _;
        for (const auto& message : messages) {
            EXPECT_CALL(*mSharedMemMock, receive)
                .WillOnce(DoAll(SetArgPointerInReceiveMessage(message), Return(true)));
        }
    }
    RestartApplication(std::chrono::milliseconds{500});
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "#0 A \n",
        "#0 A \n#1 B \n",
        "#0 A \n#1 B \n#2 C \n",
        "#0 A \n#1 B <\n#2 C \n"
    };
    EXPECT_EQ(actual, expected);
}

TEST_F(TTContactsTest, OneHeartbeatThreeNewContactsOneSelectedOneInactive) {
    EXPECT_CALL(*mSharedMemMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSharedMemMock, alive)
        .WillRepeatedly(Return(true));
    std::vector<TTContactsMessage> messages;
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_ACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 2, "C"));
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::INACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::GOODBYE));
    {
        InSequence _;
        for (const auto& message : messages) {
            EXPECT_CALL(*mSharedMemMock, receive)
                .WillOnce(DoAll(SetArgPointerInReceiveMessage(message), Return(true)));
        }
    }
    RestartApplication(std::chrono::milliseconds{500});
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "#0 A \n",
        "#0 A <\n",
        "#0 A <\n#1 B \n",
        "#0 A <\n#1 B \n#2 C \n",
        "#0 A <\n#1 B ?\n#2 C \n"
    };
    EXPECT_EQ(actual, expected);
}

TEST_F(TTContactsTest, OneHeartbeatThreeNewContactsOneSelectedOneInactiveThenAgainActive) {
    EXPECT_CALL(*mSharedMemMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSharedMemMock, alive)
        .WillRepeatedly(Return(true));
    std::vector<TTContactsMessage> messages;
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_ACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::INACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 2, "C"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::GOODBYE));
    {
        InSequence _;
        for (const auto& message : messages) {
            EXPECT_CALL(*mSharedMemMock, receive)
                .WillOnce(DoAll(SetArgPointerInReceiveMessage(message), Return(true)));
        }
    }
    RestartApplication(std::chrono::milliseconds{500});
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "#0 A \n",
        "#0 A <\n",
        "#0 A <\n#1 B \n",
        "#0 A <\n#1 B ?\n",
        "#0 A <\n#1 B ?\n#2 C \n",
        "#0 A <\n#1 B \n#2 C \n"
    };
    EXPECT_EQ(actual, expected);
}

TEST_F(TTContactsTest, OneHeartbeatThreeNewContactsOneInactiveAndSelected) {
    EXPECT_CALL(*mSharedMemMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSharedMemMock, alive)
        .WillRepeatedly(Return(true));
    std::vector<TTContactsMessage> messages;
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_ACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::INACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 2, "C"));
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_INACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::GOODBYE));
    {
        InSequence _;
        for (const auto& message : messages) {
            EXPECT_CALL(*mSharedMemMock, receive)
                .WillOnce(DoAll(SetArgPointerInReceiveMessage(message), Return(true)));
        }
    }
    RestartApplication(std::chrono::milliseconds{500});
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "#0 A \n",
        "#0 A <\n",
        "#0 A <\n#1 B \n",
        "#0 A <\n#1 B ?\n",
        "#0 A <\n#1 B ?\n#2 C \n",
        "#0 A <\n#1 B <?\n#2 C \n"
    };
    EXPECT_EQ(actual, expected);
}

TEST_F(TTContactsTest, OneHeartbeatThreeNewContactsOneSelectedTwoUnreadMessage) {
    EXPECT_CALL(*mSharedMemMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSharedMemMock, alive)
        .WillRepeatedly(Return(true));
    std::vector<TTContactsMessage> messages;
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_ACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::UNREAD_MSG_ACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 2, "C"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::UNREAD_MSG_ACTIVE, 2, "C"));
    messages.push_back(CreateMessage(TTContactsStatus::GOODBYE));
    {
        InSequence _;
        for (const auto& message : messages) {
            EXPECT_CALL(*mSharedMemMock, receive)
                .WillOnce(DoAll(SetArgPointerInReceiveMessage(message), Return(true)));
        }
    }
    RestartApplication(std::chrono::milliseconds{500});
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "#0 A \n",
        "#0 A \n#1 B \n",
        "#0 A \n#1 B <\n",
        "#0 A @\n#1 B <\n",
        "#0 A @\n#1 B <\n#2 C \n",
        "#0 A @\n#1 B <\n#2 C @\n"
    };
    EXPECT_EQ(actual, expected);
}

TEST_F(TTContactsTest, OneHeartbeatThreeNewContactsOneSelectedTwoUnreadMessageInactive) {
    EXPECT_CALL(*mSharedMemMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSharedMemMock, alive)
        .WillRepeatedly(Return(true));
    std::vector<TTContactsMessage> messages;
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_ACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::UNREAD_MSG_ACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 2, "C"));
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::UNREAD_MSG_INACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::UNREAD_MSG_ACTIVE, 2, "C"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::UNREAD_MSG_INACTIVE, 2, "C"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_ACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::GOODBYE));
    {
        InSequence _;
        for (const auto& message : messages) {
            EXPECT_CALL(*mSharedMemMock, receive)
                .WillOnce(DoAll(SetArgPointerInReceiveMessage(message), Return(true)));
        }
    }
    RestartApplication(std::chrono::milliseconds{500});
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "#0 A \n",
        "#0 A \n#1 B \n",
        "#0 A \n#1 B <\n",
        "#0 A @\n#1 B <\n",
        "#0 A @\n#1 B <\n#2 C \n",
        "#0 A @?\n#1 B <\n#2 C \n",
        "#0 A @?\n#1 B <\n#2 C @\n",
        "#0 A @?\n#1 B <\n#2 C @?\n",
        "#0 A @?\n#1 B \n#2 C @?\n",
        "#0 A <\n#1 B \n#2 C @?\n"
    };
    EXPECT_EQ(actual, expected);
}

TEST_F(TTContactsTest, OneHeartbeatThreeNewContactsOneSelectedOnePendingMessageInactive) {
    EXPECT_CALL(*mSharedMemMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSharedMemMock, alive)
        .WillRepeatedly(Return(true));
    std::vector<TTContactsMessage> messages;
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_ACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 2, "C"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_ACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_INACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_PENDING_MSG_INACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::PENDING_MSG_INACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_ACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::GOODBYE));
    {
        InSequence _;
        for (const auto& message : messages) {
            EXPECT_CALL(*mSharedMemMock, receive)
                .WillOnce(DoAll(SetArgPointerInReceiveMessage(message), Return(true)));
        }
    }
    RestartApplication(std::chrono::milliseconds{500});
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "#0 A \n",
        "#0 A <\n",
        "#0 A <\n#1 B \n",
        "#0 A <\n#1 B \n#2 C \n",
        "#0 A \n#1 B \n#2 C \n",
        "#0 A \n#1 B <\n#2 C \n",
        "#0 A \n#1 B <?\n#2 C \n",
        "#0 A \n#1 B <!?\n#2 C \n",
        "#0 A \n#1 B !?\n#2 C \n",
        "#0 A <\n#1 B !?\n#2 C \n"
    };
    EXPECT_EQ(actual, expected);
}

TEST_F(TTContactsTest, OneHeartbeatThreeNewContactsMixStates) {
    EXPECT_CALL(*mSharedMemMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSharedMemMock, alive)
        .WillRepeatedly(Return(true));
    std::vector<TTContactsMessage> messages;
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_ACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 2, "C"));
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::INACTIVE, 2, "C"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_INACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::INACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_ACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::UNREAD_MSG_ACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 2, "C"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::UNREAD_MSG_INACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::UNREAD_MSG_ACTIVE, 2, "C"));
    messages.push_back(CreateMessage(TTContactsStatus::HEARTBEAT));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_INACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_PENDING_MSG_INACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::PENDING_MSG_INACTIVE, 1, "B"));
    messages.push_back(CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_INACTIVE, 0, "A"));
    messages.push_back(CreateMessage(TTContactsStatus::GOODBYE));
    {
        InSequence _;
        for (const auto& message : messages) {
            EXPECT_CALL(*mSharedMemMock, receive)
                .WillOnce(DoAll(SetArgPointerInReceiveMessage(message), Return(true)));
        }
    }
    RestartApplication(std::chrono::milliseconds{500});
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "#0 A \n",
        "#0 A <\n",
        "#0 A <\n#1 B \n",
        "#0 A <\n#1 B \n#2 C \n",
        "#0 A <\n#1 B \n#2 C ?\n",
        "#0 A <?\n#1 B \n#2 C ?\n",
        "#0 A ?\n#1 B \n#2 C ?\n",
        "#0 A ?\n#1 B <\n#2 C ?\n",
        "#0 A \n#1 B <\n#2 C ?\n",
        "#0 A @\n#1 B <\n#2 C ?\n",
        "#0 A @\n#1 B <\n#2 C \n",
        "#0 A @?\n#1 B <\n#2 C \n",
        "#0 A @?\n#1 B <\n#2 C @\n",
        "#0 A @?\n#1 B <?\n#2 C @\n",
        "#0 A @?\n#1 B <!?\n#2 C @\n",
        "#0 A @?\n#1 B !?\n#2 C @\n",
        "#0 A <?\n#1 B !?\n#2 C @\n",
    };
    EXPECT_EQ(actual, expected);
}
