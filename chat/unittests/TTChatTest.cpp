#include "TTChat.hpp"
#include "TTChatSettingsMock.hpp"
#include "TTUtilsMessageQueueMock.hpp"
#include "TTUtilsOutputStreamMock.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <memory>
#include <vector>

using ::testing::Test;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::InSequence;
using ::testing::AtLeast;
using ::testing::_;
using namespace std::placeholders;

class TTChatTest : public Test {
public:
    void SetArgPointerInReceiveMessage(char* dst, const TTChatMessage& src, std::chrono::milliseconds timeout) {
        std::this_thread::sleep_for(timeout);
        std::memcpy(dst, &src, sizeof(src));
        mStoppedStatusOnReceive.emplace_back(mChat->isStopped());
    }

    void GetArgPointerInSendMessage(const char* src, std::chrono::milliseconds timeout) {
        std::this_thread::sleep_for(timeout);
        TTChatMessage dst;
        std::memcpy(&dst, src, sizeof(dst));
        mSendMessages.push_back(dst);
        mStoppedStatusOnSend.emplace_back(mChat->isStopped());
    }
protected:
    TTChatTest() {
        mSettingsMock = std::make_shared<TTChatSettingsMock>();
        mPrimaryMessageQueueMock = std::make_shared<TTUtilsMessageQueueMock>();
        mSecondaryMessageQueueMock = std::make_shared<TTUtilsMessageQueueMock>();
        mOutputStreamMock = std::make_shared<TTUtilsOutputStreamMock>();
    }
    ~TTChatTest() {

    }
    // Called after constructor, before each test
    virtual void SetUp() override {
        EXPECT_CALL(*mSettingsMock, getPrimaryMessageQueue)
            .Times(1)
            .WillOnce(Return(mPrimaryMessageQueueMock));
        EXPECT_CALL(*mSettingsMock, getSecondaryMessageQueue)
            .Times(1)
            .WillOnce(Return(mSecondaryMessageQueueMock));
        EXPECT_CALL(*mSettingsMock, getTerminalWidth)
            .Times(1)
            .WillOnce(Return(TERMINAL_WIDTH));
        EXPECT_CALL(*mSettingsMock, getTerminalHeight)
            .Times(1)
            .WillOnce(Return(TERMINAL_HEIGHT));
        EXPECT_CALL(*mSettingsMock, getRatio)
            .Times(1)
            .WillOnce(Return(TERMINAL_RATIO));
    }
    // Called before destructor, after each test
    virtual void TearDown() override {
        mChat.reset();
        mOutputStreamMock->mOutput.clear();
        mStoppedStatusOnSend.clear();
        mStoppedStatusOnReceive.clear();
        mSendMessages.clear();
    }

    void StartApplication() {
        mChat->run();
    }

    void RestartApplication() {
        mChat = std::make_unique<TTChat>(*mSettingsMock, *mOutputStreamMock);
        EXPECT_FALSE(mChat->isStopped());
        mChat->subscribeOnStop(mApplicationCv);
        mApplicationThread = std::thread{&TTChatTest::StartApplication, this};
    }

    void RestartApplicationNoCheck() {
        mChat = std::make_unique<TTChat>(*mSettingsMock, *mOutputStreamMock);
        mApplicationThread = std::thread{&TTChatTest::StartApplication, this};
    }

    void VerifyApplicationTimeout(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mApplicationMutex);
        const bool predicate = mApplicationCv.wait_for(lock, timeout, [this]() {
            return mChat->isStopped();
        });
        EXPECT_TRUE(predicate); // Check for application timeout
        EXPECT_TRUE(mChat->isStopped());
        mApplicationThread.join();
    }

    std::shared_ptr<TTChatSettingsMock> mSettingsMock;
    std::shared_ptr<TTUtilsMessageQueueMock> mPrimaryMessageQueueMock;
    std::shared_ptr<TTUtilsMessageQueueMock> mSecondaryMessageQueueMock;
    std::shared_ptr<TTUtilsOutputStreamMock> mOutputStreamMock;
    std::unique_ptr<TTChat> mChat;
    std::thread mApplicationThread;
    std::mutex mApplicationMutex;
    std::condition_variable mApplicationCv;
    std::vector<bool> mStoppedStatusOnSend;
    std::vector<bool> mStoppedStatusOnReceive;
    std::vector<TTChatMessage> mSendMessages;
    constexpr static size_t TERMINAL_WIDTH = 50;
    constexpr static size_t TERMINAL_HEIGHT = 50;
    constexpr static double TERMINAL_RATIO = 1.0;
    constexpr static long HEARTBEAT_TIMEOUT_MS = 500; // 0.5s
};

TEST_F(TTChatTest, FailedToOpenPrimaryMessageQueue) {
    EXPECT_CALL(*mPrimaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_THROW(RestartApplication(), std::runtime_error);
}

TEST_F(TTChatTest, FailedToOpenSecondaryMessageQueue) {
    EXPECT_CALL(*mPrimaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_THROW(RestartApplication(), std::runtime_error);
}

TEST_F(TTChatTest, FailedToRunPrimaryMessageQueueNotAlive) {
    EXPECT_CALL(*mPrimaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));
    RestartApplicationNoCheck();
    VerifyApplicationTimeout(std::chrono::milliseconds{500});
}

TEST_F(TTChatTest, FailedToRunSecondaryMessageQueueNotAlive) {
    EXPECT_CALL(*mPrimaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));
    RestartApplicationNoCheck();
    VerifyApplicationTimeout(std::chrono::milliseconds{500});
}

TEST_F(TTChatTest, FailedToReceiveAfterManyReceivedHeartbeats) {
    EXPECT_CALL(*mPrimaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    const auto heartbeat = TTChatMessage(TTChatMessageType::HEARTBEAT);
    const size_t numOfReceivedHearbeats = 15;
    const size_t minNumOfSentMessages = 4;
    const auto receiveDelayTicks = 200;
    const auto receiveDelay = std::chrono::milliseconds{receiveDelayTicks};
    const auto sendDelay = std::chrono::milliseconds{0};
    {
        InSequence _;
        EXPECT_CALL(*mPrimaryMessageQueueMock, receive)
            .Times(AtLeast(numOfReceivedHearbeats))
            
            .WillRepeatedly(DoAll(std::bind(&TTChatTest::SetArgPointerInReceiveMessage, this, _1, heartbeat, receiveDelay), Return(true)));
        EXPECT_CALL(*mPrimaryMessageQueueMock, receive)
            .Times(1)
            .WillOnce(DoAll(std::bind(&TTChatTest::SetArgPointerInReceiveMessage, this, _1, heartbeat, receiveDelay), Return(false)));
    }
    EXPECT_CALL(*mSecondaryMessageQueueMock, send)
        .Times(AtLeast(minNumOfSentMessages))
        .WillRepeatedly(DoAll(std::bind(&TTChatTest::GetArgPointerInSendMessage, this, _1, sendDelay), Return(true)));
    RestartApplication();
    VerifyApplicationTimeout(std::chrono::milliseconds{static_cast<size_t>(numOfReceivedHearbeats * receiveDelayTicks * 1.5)});
    for (const auto status : mStoppedStatusOnReceive) {
        EXPECT_FALSE(status) << "At some point application was stopped while receiving message!";
    }
    EXPECT_GT(mStoppedStatusOnSend.size(), minNumOfSentMessages);
    for (size_t i = 0; i < minNumOfSentMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnSend[i]) << "At some point application was stopped while sending message!";
    }
    for (const auto& sendMessage : mSendMessages) {
        EXPECT_EQ(sendMessage.getType(), TTChatMessageType::HEARTBEAT);
    }
}

TEST_F(TTChatTest, FailedToSendAfterManySendHeartbeats) {
    EXPECT_CALL(*mPrimaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    const auto heartbeat = TTChatMessage(TTChatMessageType::HEARTBEAT);
    const size_t numOfSentHearbeats = 5;
    const size_t minNumOfReceivedMessages = 4;
    const auto sendDelay = std::chrono::milliseconds{10};
    const auto receiveDelay = std::chrono::milliseconds{80};
    EXPECT_CALL(*mPrimaryMessageQueueMock, receive)
        .Times(AtLeast(minNumOfReceivedMessages))
        .WillRepeatedly(DoAll(std::bind(&TTChatTest::SetArgPointerInReceiveMessage, this, _1, heartbeat, receiveDelay), Return(true)));
    {
        InSequence _;
        EXPECT_CALL(*mSecondaryMessageQueueMock, send)
            .Times(numOfSentHearbeats)
            .WillRepeatedly(DoAll(std::bind(&TTChatTest::GetArgPointerInSendMessage, this, _1, sendDelay), Return(true)));
        EXPECT_CALL(*mSecondaryMessageQueueMock, send)
            .Times(1)
            .WillOnce(DoAll(std::bind(&TTChatTest::GetArgPointerInSendMessage, this, _1, sendDelay), Return(false)));
    }
    RestartApplication();
    VerifyApplicationTimeout(std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS * (numOfSentHearbeats + 1)});
    EXPECT_GT(mStoppedStatusOnSend.size(), minNumOfReceivedMessages);
    for (size_t i = 0; i < minNumOfReceivedMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnReceive[i]) << "At some point application was stopped while receiving message!";
    }
    for (const auto status : mStoppedStatusOnSend) {
        EXPECT_FALSE(status) << "At some point application was stopped while sending message!";
    }
    for (const auto& sendMessage : mSendMessages) {
        EXPECT_EQ(sendMessage.getType(), TTChatMessageType::HEARTBEAT);
    }
}

TEST_F(TTChatTest, HappyPathOnlySendAndReceivedHeartbeats) {
    // Expected calls
    EXPECT_CALL(*mPrimaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    const auto heartbeat = TTChatMessage(TTChatMessageType::HEARTBEAT);
    const size_t minNumOfReceivedMessages = 5;
    const size_t minNumOfSentMessages = 5;
    const auto receiveDelay = std::chrono::milliseconds{40};
    const auto sendDelayTicks = 10;
    const auto sendDelay = std::chrono::milliseconds{sendDelayTicks};
    EXPECT_CALL(*mPrimaryMessageQueueMock, receive)
        .Times(AtLeast(minNumOfReceivedMessages))
        .WillRepeatedly(DoAll(std::bind(&TTChatTest::SetArgPointerInReceiveMessage, this, _1, heartbeat, receiveDelay), Return(true)));
    EXPECT_CALL(*mSecondaryMessageQueueMock, send)
        .Times(AtLeast(minNumOfSentMessages))
        .WillRepeatedly(DoAll(std::bind(&TTChatTest::GetArgPointerInSendMessage, this, _1, sendDelay), Return(true)));
    // Run
    RestartApplication();
    std::this_thread::sleep_for(std::chrono::milliseconds{(HEARTBEAT_TIMEOUT_MS + sendDelayTicks) * (minNumOfSentMessages + 1)});
    mChat->stop();
    VerifyApplicationTimeout(std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS});
    // Verify
    EXPECT_GT(mStoppedStatusOnReceive.size(), minNumOfReceivedMessages);
    for (size_t i = 0; i < minNumOfReceivedMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnReceive[i]) << "At some point application was stopped while receiving message!";
    }
    EXPECT_GT(mStoppedStatusOnSend.size(), minNumOfSentMessages);
    for (size_t i = 0; i < minNumOfSentMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnSend[i]) << "At some point application was stopped while sending message!";
    }
    for (const auto& sendMessage : mSendMessages) {
        EXPECT_EQ(sendMessage.getType(), TTChatMessageType::HEARTBEAT);
    }
}

TEST_F(TTChatTest, UnhappyPathReceivedHeartbeatsThenUnknown) {
    // Expected calls
    EXPECT_CALL(*mPrimaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    const auto heartbeat = TTChatMessage(TTChatMessageType::HEARTBEAT);
    const auto unknown = TTChatMessage(static_cast<TTChatMessageType>(0xFF));
    const size_t numOfReceivedHeartbeats = 3;
    const size_t numOfReceivedUnknows = 1;
    const size_t minNumOfSentMessages = 3;
    const auto receiveDelay = std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS};
    const auto sendDelay = std::chrono::milliseconds{10};
    {
        InSequence _;
        EXPECT_CALL(*mPrimaryMessageQueueMock, receive)
            .Times(numOfReceivedHeartbeats)
            .WillRepeatedly(DoAll(std::bind(&TTChatTest::SetArgPointerInReceiveMessage, this, _1, heartbeat, receiveDelay), Return(true)));
        EXPECT_CALL(*mPrimaryMessageQueueMock, receive)
            .Times(numOfReceivedUnknows)
            .WillOnce(DoAll(std::bind(&TTChatTest::SetArgPointerInReceiveMessage, this, _1, unknown, receiveDelay), Return(true)));
    }
    EXPECT_CALL(*mSecondaryMessageQueueMock, send)
        .Times(AtLeast(minNumOfSentMessages))
        .WillRepeatedly(DoAll(std::bind(&TTChatTest::GetArgPointerInSendMessage, this, _1, sendDelay), Return(true)));
    // Run
    RestartApplication();
    VerifyApplicationTimeout(std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS * (numOfReceivedHeartbeats + numOfReceivedUnknows + 1)});
    // Verify
    EXPECT_GT(mStoppedStatusOnReceive.size(), numOfReceivedHeartbeats);
    for (size_t i = 0; i < numOfReceivedHeartbeats; ++i) {
        EXPECT_FALSE(mStoppedStatusOnReceive[i]) << "At some point application was stopped while receiving message!";
    }
    EXPECT_GT(mStoppedStatusOnSend.size(), minNumOfSentMessages);
    for (size_t i = 0; i < minNumOfSentMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnSend[i]) << "At some point application was stopped while sending message!";
    }
    for (const auto& sendMessage : mSendMessages) {
        EXPECT_EQ(sendMessage.getType(), TTChatMessageType::HEARTBEAT);
    }
}

TEST_F(TTChatTest, HappyPathReceivedHeartbeatsThenGoodbye) {
    // Expected calls
    EXPECT_CALL(*mPrimaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    const auto heartbeat = TTChatMessage(TTChatMessageType::HEARTBEAT);
    const auto goodbye = TTChatMessage(TTChatMessageType::GOODBYE);
    const size_t numOfReceivedHeartbeats = 3;
    const size_t numOfReceivedGoodbyes = 1;
    const size_t minNumOfSentMessages = 3;
    const auto receiveDelay = std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS};
    const auto sendDelay = std::chrono::milliseconds{10};
    {
        InSequence _;
        EXPECT_CALL(*mPrimaryMessageQueueMock, receive)
            .Times(numOfReceivedHeartbeats)
            .WillRepeatedly(DoAll(std::bind(&TTChatTest::SetArgPointerInReceiveMessage, this, _1, heartbeat, receiveDelay), Return(true)));
        EXPECT_CALL(*mPrimaryMessageQueueMock, receive)
            .Times(numOfReceivedGoodbyes)
            .WillOnce(DoAll(std::bind(&TTChatTest::SetArgPointerInReceiveMessage, this, _1, goodbye, receiveDelay), Return(true)));
    }
    EXPECT_CALL(*mSecondaryMessageQueueMock, send)
        .Times(AtLeast(minNumOfSentMessages))
        .WillRepeatedly(DoAll(std::bind(&TTChatTest::GetArgPointerInSendMessage, this, _1, sendDelay), Return(true)));
    // Run
    RestartApplication();
    VerifyApplicationTimeout(std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS * (numOfReceivedHeartbeats + numOfReceivedGoodbyes + 1)});
    // Verify
    EXPECT_GT(mStoppedStatusOnReceive.size(), numOfReceivedHeartbeats);
    for (size_t i = 0; i < numOfReceivedHeartbeats; ++i) {
        EXPECT_FALSE(mStoppedStatusOnReceive[i]) << "At some point application was stopped while receiving message!";
    }
    EXPECT_GT(mStoppedStatusOnSend.size(), minNumOfSentMessages);
    for (size_t i = 0; i < minNumOfSentMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnSend[i]) << "At some point application was stopped while sending message!";
    }
    for (const auto& sendMessage : mSendMessages) {
        EXPECT_EQ(sendMessage.getType(), TTChatMessageType::HEARTBEAT);
    }
}

TEST_F(TTChatTest, HappyPathReceivedMessagesAdditionalWhitespaceCharacters) {
    // Expected calls
    EXPECT_CALL(*mPrimaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    const std::vector<TTChatMessage> messagesToBeReceived = {
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::CLEAR),
        TTChatMessage(TTChatMessageType::SENDER, {}, "  \t  Hello, I'm having some additional whitespaces at the beginning"),
        TTChatMessage(TTChatMessageType::RECEIVER, {}, "Hi, I've got additional whitespaces at the end  \t  "),
        TTChatMessage(TTChatMessageType::RECEIVER, {}, "\t  \t   \t"),
        TTChatMessage(TTChatMessageType::GOODBYE)
    };
    const size_t receiveDelayTicks = 100;
    const auto receiveDelay = std::chrono::milliseconds{receiveDelayTicks};
    const auto sendDelay = std::chrono::milliseconds{0};
    const size_t minNumOfSentMessages = (messagesToBeReceived.size() * receiveDelayTicks) / HEARTBEAT_TIMEOUT_MS;
    const size_t minNumOfReceivedMessages = messagesToBeReceived.size();
    {
        InSequence _;
        for (const auto &message : messagesToBeReceived) {
            EXPECT_CALL(*mPrimaryMessageQueueMock, receive)
                .Times(1)
                .WillOnce(DoAll(std::bind(&TTChatTest::SetArgPointerInReceiveMessage, this, _1, message, receiveDelay), Return(true)));
        }
    }
    EXPECT_CALL(*mSecondaryMessageQueueMock, send)
        .Times(AtLeast(minNumOfSentMessages))
        .WillRepeatedly(DoAll(std::bind(&TTChatTest::GetArgPointerInSendMessage, this, _1, sendDelay), Return(true)));
    // Run
    RestartApplication();
    VerifyApplicationTimeout(std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS * (minNumOfSentMessages + 1)});
    // Verify
    EXPECT_GE(mStoppedStatusOnReceive.size(), minNumOfReceivedMessages);
    for (size_t i = 0; i < minNumOfReceivedMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnReceive[i]) << "At some point application was stopped while receiving message!";
    }
    EXPECT_GT(mStoppedStatusOnSend.size(), minNumOfSentMessages);
    for (size_t i = 0; i < minNumOfSentMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnSend[i]) << "At some point application was stopped while sending message!";
    }
    for (const auto& sendMessage : mSendMessages) {
        EXPECT_EQ(sendMessage.getType(), TTChatMessageType::HEARTBEAT);
    }
    const std::string conversation1 =
        std::string{"                               1970-01-01 01:00:00\n"} +
        std::string{"  Hello, I'm having some additional whitespaces at\n"} +
        std::string{"                                     the beginning\n"} +
        std::string{"\n"} +
        std::string{"1970-01-01 01:00:00\n"} + 
        std::string{"Hi, I've got additional whitespaces at the end\n"} +
        std::string{"\n"} +
        std::string{"1970-01-01 01:00:00\n"} + 
        std::string{" \n"} +
        std::string{"\n"};
    const auto& expected = std::vector<std::string>{conversation1};
    const auto& actual = mOutputStreamMock->mOutput;
    EXPECT_EQ(actual, expected);
}

TEST_F(TTChatTest, HappyPathReceivedMessagesDuplicatedWhitespaceCharacters) {
    // Expected calls
    EXPECT_CALL(*mPrimaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    const std::vector<TTChatMessage> messagesToBeReceived = {
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::CLEAR),
        TTChatMessage(TTChatMessageType::SENDER, {}, "Hello, \t I'm  having some \t\tduplicated whitespaces  in  between"),
        TTChatMessage(TTChatMessageType::RECEIVER, {}, "Hi,   \t\t\t   I've got duplicated         whitespaces in between     too"),
        TTChatMessage(TTChatMessageType::GOODBYE)
    };
    const size_t receiveDelayTicks = 100;
    const auto receiveDelay = std::chrono::milliseconds{receiveDelayTicks};
    const auto sendDelay = std::chrono::milliseconds{0};
    const size_t minNumOfSentMessages = (messagesToBeReceived.size() * receiveDelayTicks) / HEARTBEAT_TIMEOUT_MS;
    const size_t minNumOfReceivedMessages = messagesToBeReceived.size();
    {
        InSequence _;
        for (const auto &message : messagesToBeReceived) {
            EXPECT_CALL(*mPrimaryMessageQueueMock, receive)
                .Times(1)
                .WillOnce(DoAll(std::bind(&TTChatTest::SetArgPointerInReceiveMessage, this, _1, message, receiveDelay), Return(true)));
        }
    }
    EXPECT_CALL(*mSecondaryMessageQueueMock, send)
        .Times(AtLeast(minNumOfSentMessages))
        .WillRepeatedly(DoAll(std::bind(&TTChatTest::GetArgPointerInSendMessage, this, _1, sendDelay), Return(true)));
    // Run
    RestartApplication();
    VerifyApplicationTimeout(std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS * (minNumOfSentMessages + 1)});
    // Verify
    EXPECT_GE(mStoppedStatusOnReceive.size(), minNumOfReceivedMessages);
    for (size_t i = 0; i < minNumOfReceivedMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnReceive[i]) << "At some point application was stopped while receiving message!";
    }
    EXPECT_GT(mStoppedStatusOnSend.size(), minNumOfSentMessages);
    for (size_t i = 0; i < minNumOfSentMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnSend[i]) << "At some point application was stopped while sending message!";
    }
    for (const auto& sendMessage : mSendMessages) {
        EXPECT_EQ(sendMessage.getType(), TTChatMessageType::HEARTBEAT);
    }
    const std::string conversation1 =
        std::string{"                               1970-01-01 01:00:00\n"} +
        std::string{"  Hello, I'm having some duplicated whitespaces in\n"} +
        std::string{"                                           between\n"} +
        std::string{"\n"} +
        std::string{"1970-01-01 01:00:00\n"} + 
        std::string{"Hi, I've got duplicated whitespaces in between too\n"} +
        std::string{"\n"};
    const auto& expected = std::vector<std::string>{conversation1};
    const auto& actual = mOutputStreamMock->mOutput;
    EXPECT_EQ(actual, expected);
}

TEST_F(TTChatTest, HappyPathReceivedMessagesWithLongWords) {
    // Expected calls
    EXPECT_CALL(*mPrimaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    const std::vector<TTChatMessage> messagesToBeReceived = {
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::CLEAR),
        TTChatMessage(TTChatMessageType::SENDER, {}, "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                            "Suspendisse interdum imperdiet pharetra. Morbi blandit sapien at sapien vehicula blandit. "
                            "Morbi at risus mollis leo semper semper. Nulla facilisi. Donec id bibendum odio, eu pellentesque est. "
                            "Vivamus sem ipsum, aliquet sed tortor sit amet, vestibulum elementum nulla. "
                            "Duis porta porta erat, et maximus urna ullamcorper sit amet. Praesent quis molestie est. "
                            "Integer elit turpis, ullamcorper non dolor non, iaculis placerat quam. "
                            "Pellentesque lobortis elementum erat eget egestas. Integer malesuada tempor convallis. "
                            "Sed eu pretium justo. Nam tincidunt scelerisque porttitor. "
                            "Vivamus massa eros, mollis a laoreet accumsan, auctor ac ante. "
                            "Sed maximus tempor dignissim. Mauris a condimentum eros."),
        TTChatMessage(TTChatMessageType::RECEIVER, {}, "Cras semper mi at mauris porttitor accumsan. Proin vel nisi et diam dapibus tristique at at odio. "
                              "Sed mollis massa sit amet metus consectetur efficitur. "
                              "Pellentesque tristique massa erat, id pretium quam viverra eget. "
                              "Proin eget arcu consequat, posuere metus nec, porttitor mauris. "
                              "Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. "
                              "Suspendisse sem lorem, iaculis a convallis a, sagittis et arcu. "
                              "Pellentesque vitae neque ut ante porta gravida. Pellentesque pretium vehicula consectetur. "
                              "In sed imperdiet eros. Duis sagittis scelerisque nunc vitae ornare. "
                              "Sed quis sapien eu metus vulputate auctor."),
        TTChatMessage(TTChatMessageType::CLEAR),
        TTChatMessage(TTChatMessageType::SENDER, {}, "Pneumonoultramicroscopicsilicovolcanoconiosis, Pseudopseudohypoparathyroidism, Floccinaucinihilipilification, Antidisestablishmentarianism"),
        TTChatMessage(TTChatMessageType::RECEIVER, {}, "Supercalifragilisticexpialidocious, Strengths, Euouae, Unimaginatively, Honorificabilitudinitatibus, Tsktsk, Sesquipedalianism, Uncopyrightable"),
        TTChatMessage(TTChatMessageType::CLEAR),
        TTChatMessage(TTChatMessageType::RECEIVER, {}, "VeryLongMessageTakingMoreThanTotalSideWidthBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlah"),
        TTChatMessage(TTChatMessageType::SENDER, {}, "VeryLongMessageTakingMoreThanTotalSideWidthBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlah"),
        TTChatMessage(TTChatMessageType::GOODBYE)
    };
    const size_t receiveDelayTicks = 100;
    const auto receiveDelay = std::chrono::milliseconds{receiveDelayTicks};
    const auto sendDelay = std::chrono::milliseconds{0};
    const size_t minNumOfSentMessages = (messagesToBeReceived.size() * receiveDelayTicks) / HEARTBEAT_TIMEOUT_MS;
    const size_t minNumOfReceivedMessages = messagesToBeReceived.size();
    {
        InSequence _;
        for (const auto &message : messagesToBeReceived) {
            EXPECT_CALL(*mPrimaryMessageQueueMock, receive)
                .Times(1)
                .WillOnce(DoAll(std::bind(&TTChatTest::SetArgPointerInReceiveMessage, this, _1, message, receiveDelay), Return(true)));
        }
    }
    EXPECT_CALL(*mSecondaryMessageQueueMock, send)
        .Times(AtLeast(minNumOfSentMessages))
        .WillRepeatedly(DoAll(std::bind(&TTChatTest::GetArgPointerInSendMessage, this, _1, sendDelay), Return(true)));
    // Run
    RestartApplication();
    VerifyApplicationTimeout(std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS * (minNumOfSentMessages + 1)});
    // Verify
    EXPECT_GE(mStoppedStatusOnReceive.size(), minNumOfReceivedMessages);
    for (size_t i = 0; i < minNumOfReceivedMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnReceive[i]) << "At some point application was stopped while receiving message!";
    }
    EXPECT_GT(mStoppedStatusOnSend.size(), minNumOfSentMessages);
    for (size_t i = 0; i < minNumOfSentMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnSend[i]) << "At some point application was stopped while sending message!";
    }
    for (const auto& sendMessage : mSendMessages) {
        EXPECT_EQ(sendMessage.getType(), TTChatMessageType::HEARTBEAT);
    }
    const std::string conversation1 =
        std::string{"                               1970-01-01 01:00:00\n"} +
        std::string{"Lorem ipsum dolor sit amet, consectetur adipiscing\n"} +
        std::string{"    elit. Suspendisse interdum imperdiet pharetra.\n"} +
        std::string{"  Morbi blandit sapien at sapien vehicula blandit.\n"} +
        std::string{"    Morbi at risus mollis leo semper semper. Nulla\n"} +
        std::string{" facilisi. Donec id bibendum odio, eu pellentesque\n"} +
        std::string{"    est. Vivamus sem ipsum, aliquet sed tortor sit\n"} +
        std::string{"amet, vestibulum elementum nulla. Duis porta porta\n"} +
        std::string{"       erat, et maximus urna ullamcorper sit amet.\n"} +
        std::string{"  Praesent quis molestie est. Integer elit turpis,\n"} +
        std::string{" ullamcorper non dolor non, iaculis placerat quam.\n"} +
        std::string{"Pellentesque lobortis elementum erat eget egestas.\n"} +
        std::string{"Integer malesuada tempor convallis. Sed eu pretium\n"} +
        std::string{"       justo. Nam tincidunt scelerisque porttitor.\n"} +
        std::string{"    Vivamus massa eros, mollis a laoreet accumsan,\n"} +
        std::string{"     auctor ac ante. Sed maximus tempor dignissim.\n"} +
        std::string{"                        Mauris a condimentum eros.\n"} +
        std::string{"\n"} +
        std::string{"1970-01-01 01:00:00\n"} +
        std::string{"Cras semper mi at mauris porttitor accumsan. Proin\n"} +
        std::string{"vel nisi et diam dapibus tristique at at odio. Sed\n"} +
        std::string{"mollis massa sit amet metus consectetur efficitur.\n"} +
        std::string{"Pellentesque tristique massa erat, id pretium quam\n"} +
        std::string{"viverra eget. Proin eget arcu consequat, posuere\n"} +
        std::string{"metus nec, porttitor mauris. Pellentesque habitant\n"} +
        std::string{"morbi tristique senectus et netus et malesuada\n"} +
        std::string{"fames ac turpis egestas. Suspendisse sem lorem,\n"} +
        std::string{"iaculis a convallis a, sagittis et arcu.\n"} +
        std::string{"Pellentesque vitae neque ut ante porta gravida.\n"} +
        std::string{"Pellentesque pretium vehicula consectetur. In sed\n"} +
        std::string{"imperdiet eros. Duis sagittis scelerisque nunc\n"} +
        std::string{"vitae ornare. Sed quis sapien eu metus vulputate\n"} +
        std::string{"auctor.\n"} +
        std::string{"\n"};
    const std::string conversation2 =
        std::string{"                               1970-01-01 01:00:00\n"} +
        std::string{"    Pneumonoultramicroscopicsilicovolcanoconiosis,\n"} +
        std::string{"                   Pseudopseudohypoparathyroidism,\n"} +
        std::string{"                    Floccinaucinihilipilification,\n"} +
        std::string{"                      Antidisestablishmentarianism\n"} +
        std::string{"\n"} +
        std::string{"1970-01-01 01:00:00\n"} +
        std::string{"Supercalifragilisticexpialidocious, Strengths,\n"} +
        std::string{"Euouae, Unimaginatively,\n"} +
        std::string{"Honorificabilitudinitatibus, Tsktsk,\n"} +
        std::string{"Sesquipedalianism, Uncopyrightable\n"} +
        std::string{"\n"};
    const std::string conversation3 =
        std::string{"1970-01-01 01:00:00\n"} +
        std::string{"VeryLongMessageTakingMoreThanTotalSideWidthBlahBla\n"} +
        std::string{"hBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahB\n"} +
        std::string{"lahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlah\n"} +
        std::string{"\n"} +
        std::string{"                               1970-01-01 01:00:00\n"} +
        std::string{"VeryLongMessageTakingMoreThanTotalSideWidthBlahBla\n"} +
        std::string{"hBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahB\n"} +
        std::string{"   lahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlah\n"} +
        std::string{"\n"};
    const auto& expected = std::vector<std::string>{conversation1, conversation2, conversation3};
    const auto& actual = mOutputStreamMock->mOutput;
    EXPECT_EQ(actual, expected);
}

TEST_F(TTChatTest, HappyPathReceivedMessagesDifferentSystemTime) {
    // Expected calls
    EXPECT_CALL(*mPrimaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    const std::time_t epoch1 = 8555226;
    const std::time_t epoch2 = 8555227;
    const std::vector<TTChatMessage> messagesToBeReceived = {
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::CLEAR),
        TTChatMessage(TTChatMessageType::SENDER, std::chrono::system_clock::from_time_t(epoch1), "Hello John"),
        TTChatMessage(TTChatMessageType::RECEIVER, std::chrono::system_clock::from_time_t(epoch2), "Hi Mike"),
        TTChatMessage(TTChatMessageType::GOODBYE)
    };
    const size_t receiveDelayTicks = 100;
    const auto receiveDelay = std::chrono::milliseconds{receiveDelayTicks};
    const auto sendDelay = std::chrono::milliseconds{0};
    const size_t minNumOfSentMessages = (messagesToBeReceived.size() * receiveDelayTicks) / HEARTBEAT_TIMEOUT_MS;
    const size_t minNumOfReceivedMessages = messagesToBeReceived.size();
    {
        InSequence _;
        for (const auto &message : messagesToBeReceived) {
            EXPECT_CALL(*mPrimaryMessageQueueMock, receive)
                .Times(1)
                .WillOnce(DoAll(std::bind(&TTChatTest::SetArgPointerInReceiveMessage, this, _1, message, receiveDelay), Return(true)));
        }
    }
    EXPECT_CALL(*mSecondaryMessageQueueMock, send)
        .Times(AtLeast(minNumOfSentMessages))
        .WillRepeatedly(DoAll(std::bind(&TTChatTest::GetArgPointerInSendMessage, this, _1, sendDelay), Return(true)));
    // Run
    RestartApplication();
    VerifyApplicationTimeout(std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS * (minNumOfSentMessages + 1)});
    // Verify
    EXPECT_GE(mStoppedStatusOnReceive.size(), minNumOfReceivedMessages);
    for (size_t i = 0; i < minNumOfReceivedMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnReceive[i]) << "At some point application was stopped while receiving message!";
    }
    EXPECT_GT(mStoppedStatusOnSend.size(), minNumOfSentMessages);
    for (size_t i = 0; i < minNumOfSentMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnSend[i]) << "At some point application was stopped while sending message!";
    }
    for (const auto& sendMessage : mSendMessages) {
        EXPECT_EQ(sendMessage.getType(), TTChatMessageType::HEARTBEAT);
    }
    const std::string conversation1 =
        std::string{"                               1970-04-10 01:27:06\n"} +
        std::string{"                                        Hello John\n"} +
        std::string{"\n"} +
        std::string{"1970-04-10 01:27:07\n"} + 
        std::string{"Hi Mike\n"} +
        std::string{"\n"};
    const auto& expected = std::vector<std::string>{conversation1};
    const auto& actual = mOutputStreamMock->mOutput;
    EXPECT_EQ(actual, expected);
}

TEST_F(TTChatTest, HappyPathReceivedChunkMessages) {
    // Expected strings
    const std::string longMessage1Chunk1 =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras euismod vitae mi et molestie."
        "Sed ultricies, nibh vel tincidunt accumsan, ante eros ullamcorper ipsum, feugiat consectetur mi eros sit amet nulla."
        "Cras non ligula odio. Nullam non mattis lorem. Duis a dictum lacus. Aliquam varius cursus leo id scelerisque."
        "Phasellus nec odio vestibulum, scelerisque nisl id, ornare risus."
        "Curabitur porta sem quis augue mattis, nec pulvinar sapien fermentum."
        "Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Sed congue blandit magna."
        "In ut lectus facilisis, varius dolor sed, luctus turpis. Quisque blandit scelerisque convallis."
        "Fusce ac sem eros. Aliquam faucibus viverra dapibus. Fusce quis condimentum lacus, vel gravida justo."
        "Nam quis nunc sed libero pretium lobortis nec ut arcu."
        "Duis egestas nisl nec dictum porttitor. In hac habitasse platea dictumst."
        "Nullam rhoncus euismod lorem, a tristique nunc ullamcorper scelerisque."
        "Aliquam eget mauris eu metus sodales feugiat. Vivamus vel consectetur justo, a posuere nulla."
        "Fusce vulputate consectetur orci, a tincidunt sapien condimentum efficitur."
        "Maecenas vel libero ut lacus aliquam porttitor vehicula at enim. Morbi sed massa arcu. Curabitur pharetra sodales laoreet."
        "Ut rutrum erat nulla, nec tincidunt nisi feugiat sed."
        "Aliquam tincidunt metus elit, efficitur malesuada urna aliquam eu. Donec vitae pulvinar mauris."
        "Phasellus eleifend porttitor maximus. Morbi purus orci, pulvinar a semper et, lacinia quis nunc."
        "Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae."
        "Vivamus nec tortor nec mauris porttitor elementum. Proin finibus dignissim nulla, quis lobortis lectus interdum id."
        "Donec eu sapien ultricies risus ornare aliquet eget eget diam."
        "Proin eu neque aliquet, ultrices felis in, dictum eros. Interdum et malesuada fames ac ante ipsum primis in faucibus."
        "Quisque ligula erat, luctus non consequat vitae, lobortis non ligula. Sed vehicula lectus eu blandit feugiat."
        "Aenean pulvinar erat at odio fermentum semper. Nullam nec metus lor";
    const std::string longMessage1Chunk2 =
        "em. Integer malesuada felis a arcu consectetur ornare."
        "In condimentum volutpat felis vel venenatis. Nam feugiat arcu non urna tempus hendrerit."
        "Duis et enim ex. Nulla posuere enim ut urna consequat commodo. Ut lobortis quam non arcu rutrum, quis ornare ipsum hendrerit."
        "Etiam dictum auctor rutrum. Phasellus justo urna, consequat vel elementum vitae, auctor mattis massa."
        "Proin accumsan vitae libero convallis tincidunt. Pellentesque convallis venenatis tortor, vel placerat nunc tempor a."
        "Aliquam euismod eget augue eu ultricies. Suspendisse potenti. In hac habitasse platea dictumst."
        "Fusce eget arcu ut tellus tristique tincidunt. Phasellus commodo ut quam placerat efficitur."
        "Mauris felis diam, lobortis nec consequat eget, ornare at turpis. Cras eget sem a felis tincidunt interdum at a quam.";
    const std::string longMessage2Chunk1 =
        "Curabitur lacinia augue ac massa rutrum vehicula."
        "Duis hendrerit, magna et tristique pharetra, orci metus ornare arcu, egestas blandit lectus ex in libero."
        "Morbi at dui pharetra, accumsan lacus in, maximus ligula. Donec id commodo lectus, vel rutrum nibh."
        "Vestibulum laoreet aliquet ultrices. Quisque commodo diam vitae placerat interdum. Integer vestibulum vitae ex et tincidunt."
        "Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas."
        "Nam maximus velit vitae lectus fermentum porta non ac ex. In aliquet iaculis luctus. Mauris sed urna ipsum."
        "Suspendisse id ultrices felis, vitae semper est. Pellentesque laoreet quam et erat sollicitudin semper."
        "Nullam rhoncus orci tempus orci placerat, eleifend venenatis erat porta. Ut convallis ante eu efficitur suscipit."
        "Morbi rhoncus, neque eget tincidunt porta, tortor purus venenatis ipsum, et malesuada libero ipsum non metus."
        "Fusce non malesuada diam, quis maximus odio. In vitae tincidunt mauris. Ut faucibus vel erat vitae luctus."
        "Suspendisse vel ligula id ipsum condimentum condimentum. Sed et lectus ultricies, lacinia dui vitae, fringilla turpis."
        "Quisque fermentum quam vel porttitor auctor. In hac habitasse platea dictumst. Ut id sem felis."
        "Duis laoreet at ligula ac pellentesque. Nullam nunc turpis, accumsan eget vehicula quis, facilisis sed ligula."
        "Aenean et nulla mollis, posuere lectus non, sodales neque. Nulla elementum pharetra magna quis egestas."
        "Aliquam erat volutpat. Sed est nunc, interdum et imperdiet et, feugiat a ex."
        "Nulla hendrerit placerat elit, sit amet pellentesque nibh vulputate non."
        "Integer dictum sapien vitae sapien rutrum pellentesque. Nulla egestas dolor nisl, mollis euismod purus sodales nec."
        "Aliquam vel diam vel magna tempor vulputate. Pellentesque a diam at nisi suscipit auctor ut rhoncus ex."
        "Morbi ut vehicula magna. Nulla ac metus feugiat, auctor arcu ut, imperdiet dolor."
        "Donec in sollicitudin urna, vitae convallis nisl. Duis id dolor feugiat, dignissim dui suscipit, consequat orci."
        "Pellentesque vestibulum ante vitae neque dapibus tempor";
    const std::string longMessage2Chunk2 =
        ". Phasellus elementum augue nisi, ac mattis tellus commodo id."
        "Nam vehicula rutrum eros a auctor. Interdum et malesuada fames ac ante ipsum primis in faucibus."
        "Donec ut arcu id massa porttitor suscipit. Aliquam ullamcorper vulputate elit vel tempus."
        "Donec purus lacus, dictum vel lectus nec, tincidunt consequat arcu."
        "Quisque purus justo, sollicitudin at posuere nec, semper ut quam. Cras rutrum sollicitudin justo ac lobortis."
        "Quisque placerat, augue at sagittis lacinia, sem lectus pellentesque velit, et finibus diam tellus a enim."
        "Nam in fringilla turpis. Vestibulum pellentesque felis nec ex consectetur, non pulvinar lacus facilisis."
        "Morbi lobortis metus ut tincidunt ullamcorper.";
    const std::string longMessage3Chunk1(2048, 'x');
    const std::string longMessage3Chunk2(2048, 'x');
    const std::string longMessage3Chunk3(2048, 'x');
    const std::string longMessage3Chunk4(2048, 'x');
    const std::string longMessage3Chunk5(696, 'x');
    const std::string longMessage4Chunk1(2047, 'y');
    const std::string longMessage5Chunk1(2048, 'z');
    const std::string longMessage5Chunk2(1, 'z');
    const std::string longMessage6Chunk1(2048, 'a');
    const std::string longMessage1 = longMessage1Chunk1 + longMessage1Chunk2;
    const std::string longMessage2 = longMessage2Chunk1 + longMessage2Chunk2;
    const std::string longMessage3 = longMessage3Chunk1 + longMessage3Chunk2 + longMessage3Chunk3 + longMessage3Chunk4 + longMessage3Chunk5;
    const std::string longMessage4 = longMessage4Chunk1;
    const std::string longMessage5 = longMessage5Chunk1 + longMessage5Chunk2;
    const std::string longMessage6 = longMessage6Chunk1;
    // Expected calls
    EXPECT_CALL(*mPrimaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    const std::vector<TTChatMessage> messagesToBeReceived = {
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::CLEAR),
        TTChatMessage(TTChatMessageType::SENDER_CHUNK, {}, longMessage1Chunk1),
        TTChatMessage(TTChatMessageType::SENDER, {}, longMessage1Chunk2),
        TTChatMessage(TTChatMessageType::RECEIVER_CHUNK, {}, longMessage2Chunk1),
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::RECEIVER, {}, longMessage2Chunk2),
        TTChatMessage(TTChatMessageType::CLEAR),
        TTChatMessage(TTChatMessageType::RECEIVER_CHUNK, {}, longMessage3Chunk1),
        TTChatMessage(TTChatMessageType::RECEIVER_CHUNK, {}, longMessage3Chunk2),
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::RECEIVER_CHUNK, {}, longMessage3Chunk3),
        TTChatMessage(TTChatMessageType::RECEIVER_CHUNK, {}, longMessage3Chunk4),
        TTChatMessage(TTChatMessageType::RECEIVER, {}, longMessage3Chunk5),
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::RECEIVER, {}, longMessage4Chunk1),
        TTChatMessage(TTChatMessageType::SENDER_CHUNK, {}, longMessage5Chunk1),
        TTChatMessage(TTChatMessageType::SENDER, {}, longMessage5Chunk2),
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::SENDER, {}, longMessage6Chunk1),
        TTChatMessage(TTChatMessageType::GOODBYE)
    };
    const size_t receiveDelayTicks = 100;
    const auto receiveDelay = std::chrono::milliseconds{receiveDelayTicks};
    const auto sendDelay = std::chrono::milliseconds{0};
    const size_t minNumOfSentMessages = (messagesToBeReceived.size() * receiveDelayTicks) / HEARTBEAT_TIMEOUT_MS;
    const size_t minNumOfReceivedMessages = messagesToBeReceived.size();
    {
        InSequence _;
        for (const auto &message : messagesToBeReceived) {
            EXPECT_CALL(*mPrimaryMessageQueueMock, receive)
                .Times(1)
                .WillOnce(DoAll(std::bind(&TTChatTest::SetArgPointerInReceiveMessage, this, _1, message, receiveDelay), Return(true)));
        }
    }
    EXPECT_CALL(*mSecondaryMessageQueueMock, send)
        .Times(AtLeast(minNumOfSentMessages))
        .WillRepeatedly(DoAll(std::bind(&TTChatTest::GetArgPointerInSendMessage, this, _1, sendDelay), Return(true)));
    // Run
    RestartApplication();
    VerifyApplicationTimeout(std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS * (minNumOfSentMessages + 1 + 1)});
    // Verify
    EXPECT_GE(mStoppedStatusOnReceive.size(), minNumOfReceivedMessages);
    for (size_t i = 0; i < minNumOfReceivedMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnReceive[i]) << "At some point application was stopped while receiving message!";
    }
    EXPECT_GT(mStoppedStatusOnSend.size(), minNumOfSentMessages);
    for (size_t i = 0; i < minNumOfSentMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnSend[i]) << "At some point application was stopped while sending message!";
    }
    for (const auto& sendMessage : mSendMessages) {
        EXPECT_EQ(sendMessage.getType(), TTChatMessageType::HEARTBEAT);
    }
    const std::string conversation1 = 
        std::string("                               1970-01-01 01:00:00\n") +
        std::string("Lorem ipsum dolor sit amet, consectetur adipiscing\n") +
        std::string("       elit. Cras euismod vitae mi et molestie.Sed\n") +
        std::string(" ultricies, nibh vel tincidunt accumsan, ante eros\n") +
        std::string("ullamcorper ipsum, feugiat consectetur mi eros sit\n") +
        std::string("amet nulla.Cras non ligula odio. Nullam non mattis\n") +
        std::string(" lorem. Duis a dictum lacus. Aliquam varius cursus\n") +
        std::string(" leo id scelerisque.Phasellus nec odio vestibulum,\n") +
        std::string(" scelerisque nisl id, ornare risus.Curabitur porta\n") +
        std::string("        sem quis augue mattis, nec pulvinar sapien\n") +
        std::string("fermentum.Vestibulum ante ipsum primis in faucibus\n") +
        std::string("orci luctus et ultrices posuere cubilia curae; Sed\n") +
        std::string("      congue blandit magna.In ut lectus facilisis,\n") +
        std::string("  varius dolor sed, luctus turpis. Quisque blandit\n") +
        std::string("  scelerisque convallis.Fusce ac sem eros. Aliquam\n") +
        std::string("  faucibus viverra dapibus. Fusce quis condimentum\n") +
        std::string(" lacus, vel gravida justo.Nam quis nunc sed libero\n") +
        std::string("pretium lobortis nec ut arcu.Duis egestas nisl nec\n") +
        std::string("         dictum porttitor. In hac habitasse platea\n") +
        std::string("dictumst.Nullam rhoncus euismod lorem, a tristique\n") +
        std::string("  nunc ullamcorper scelerisque.Aliquam eget mauris\n") +
        std::string(" eu metus sodales feugiat. Vivamus vel consectetur\n") +
        std::string("justo, a posuere nulla.Fusce vulputate consectetur\n") +
        std::string("              orci, a tincidunt sapien condimentum\n") +
        std::string("    efficitur.Maecenas vel libero ut lacus aliquam\n") +
        std::string(" porttitor vehicula at enim. Morbi sed massa arcu.\n") +
        std::string(" Curabitur pharetra sodales laoreet.Ut rutrum erat\n") +
        std::string("     nulla, nec tincidunt nisi feugiat sed.Aliquam\n") +
        std::string("    tincidunt metus elit, efficitur malesuada urna\n") +
        std::string(" aliquam eu. Donec vitae pulvinar mauris.Phasellus\n") +
        std::string("     eleifend porttitor maximus. Morbi purus orci,\n") +
        std::string("pulvinar a semper et, lacinia quis nunc.Vestibulum\n") +
        std::string("      ante ipsum primis in faucibus orci luctus et\n") +
        std::string(" ultrices posuere cubilia curae.Vivamus nec tortor\n") +
        std::string("     nec mauris porttitor elementum. Proin finibus\n") +
        std::string("    dignissim nulla, quis lobortis lectus interdum\n") +
        std::string(" id.Donec eu sapien ultricies risus ornare aliquet\n") +
        std::string("   eget eget diam.Proin eu neque aliquet, ultrices\n") +
        std::string("felis in, dictum eros. Interdum et malesuada fames\n") +
        std::string("   ac ante ipsum primis in faucibus.Quisque ligula\n") +
        std::string("    erat, luctus non consequat vitae, lobortis non\n") +
        std::string("            ligula. Sed vehicula lectus eu blandit\n") +
        std::string("    feugiat.Aenean pulvinar erat at odio fermentum\n") +
        std::string(" semper. Nullam nec metus lorem. Integer malesuada\n") +
        std::string("    felis a arcu consectetur ornare.In condimentum\n") +
        std::string("volutpat felis vel venenatis. Nam feugiat arcu non\n") +
        std::string("      urna tempus hendrerit.Duis et enim ex. Nulla\n") +
        std::string("        posuere enim ut urna consequat commodo. Ut\n") +
        std::string("  lobortis quam non arcu rutrum, quis ornare ipsum\n") +
        std::string("   hendrerit.Etiam dictum auctor rutrum. Phasellus\n") +
        std::string(" justo urna, consequat vel elementum vitae, auctor\n") +
        std::string("mattis massa.Proin accumsan vitae libero convallis\n") +
        std::string("       tincidunt. Pellentesque convallis venenatis\n") +
        std::string("tortor, vel placerat nunc tempor a.Aliquam euismod\n") +
        std::string("  eget augue eu ultricies. Suspendisse potenti. In\n") +
        std::string("  hac habitasse platea dictumst.Fusce eget arcu ut\n") +
        std::string("  tellus tristique tincidunt. Phasellus commodo ut\n") +
        std::string("        quam placerat efficitur.Mauris felis diam,\n") +
        std::string("    lobortis nec consequat eget, ornare at turpis.\n") +
        std::string("     Cras eget sem a felis tincidunt interdum at a\n") +
        std::string("                                             quam.\n") +
        std::string("\n") +
        std::string("1970-01-01 01:00:00\n") +
        std::string("Curabitur lacinia augue ac massa rutrum\n") +
        std::string("vehicula.Duis hendrerit, magna et tristique\n") +
        std::string("pharetra, orci metus ornare arcu, egestas blandit\n") +
        std::string("lectus ex in libero.Morbi at dui pharetra,\n") +
        std::string("accumsan lacus in, maximus ligula. Donec id\n") +
        std::string("commodo lectus, vel rutrum nibh.Vestibulum laoreet\n") +
        std::string("aliquet ultrices. Quisque commodo diam vitae\n") +
        std::string("placerat interdum. Integer vestibulum vitae ex et\n") +
        std::string("tincidunt.Pellentesque habitant morbi tristique\n") +
        std::string("senectus et netus et malesuada fames ac turpis\n") +
        std::string("egestas.Nam maximus velit vitae lectus fermentum\n") +
        std::string("porta non ac ex. In aliquet iaculis luctus. Mauris\n") +
        std::string("sed urna ipsum.Suspendisse id ultrices felis,\n") +
        std::string("vitae semper est. Pellentesque laoreet quam et\n") +
        std::string("erat sollicitudin semper.Nullam rhoncus orci\n") +
        std::string("tempus orci placerat, eleifend venenatis erat\n") +
        std::string("porta. Ut convallis ante eu efficitur\n") +
        std::string("suscipit.Morbi rhoncus, neque eget tincidunt\n") +
        std::string("porta, tortor purus venenatis ipsum, et malesuada\n") +
        std::string("libero ipsum non metus.Fusce non malesuada diam,\n") +
        std::string("quis maximus odio. In vitae tincidunt mauris. Ut\n") +
        std::string("faucibus vel erat vitae luctus.Suspendisse vel\n") +
        std::string("ligula id ipsum condimentum condimentum. Sed et\n") +
        std::string("lectus ultricies, lacinia dui vitae, fringilla\n") +
        std::string("turpis.Quisque fermentum quam vel porttitor\n") +
        std::string("auctor. In hac habitasse platea dictumst. Ut id\n") +
        std::string("sem felis.Duis laoreet at ligula ac pellentesque.\n") +
        std::string("Nullam nunc turpis, accumsan eget vehicula quis,\n") +
        std::string("facilisis sed ligula.Aenean et nulla mollis,\n") +
        std::string("posuere lectus non, sodales neque. Nulla elementum\n") +
        std::string("pharetra magna quis egestas.Aliquam erat volutpat.\n") +
        std::string("Sed est nunc, interdum et imperdiet et, feugiat a\n") +
        std::string("ex.Nulla hendrerit placerat elit, sit amet\n") +
        std::string("pellentesque nibh vulputate non.Integer dictum\n") +
        std::string("sapien vitae sapien rutrum pellentesque. Nulla\n") +
        std::string("egestas dolor nisl, mollis euismod purus sodales\n") +
        std::string("nec.Aliquam vel diam vel magna tempor vulputate.\n") +
        std::string("Pellentesque a diam at nisi suscipit auctor ut\n") +
        std::string("rhoncus ex.Morbi ut vehicula magna. Nulla ac metus\n") +
        std::string("feugiat, auctor arcu ut, imperdiet dolor.Donec in\n") +
        std::string("sollicitudin urna, vitae convallis nisl. Duis id\n") +
        std::string("dolor feugiat, dignissim dui suscipit, consequat\n") +
        std::string("orci.Pellentesque vestibulum ante vitae neque\n") +
        std::string("dapibus tempor. Phasellus elementum augue nisi, ac\n") +
        std::string("mattis tellus commodo id.Nam vehicula rutrum eros\n") +
        std::string("a auctor. Interdum et malesuada fames ac ante\n") +
        std::string("ipsum primis in faucibus.Donec ut arcu id massa\n") +
        std::string("porttitor suscipit. Aliquam ullamcorper vulputate\n") +
        std::string("elit vel tempus.Donec purus lacus, dictum vel\n") +
        std::string("lectus nec, tincidunt consequat arcu.Quisque purus\n") +
        std::string("justo, sollicitudin at posuere nec, semper ut\n") +
        std::string("quam. Cras rutrum sollicitudin justo ac\n") +
        std::string("lobortis.Quisque placerat, augue at sagittis\n") +
        std::string("lacinia, sem lectus pellentesque velit, et finibus\n") +
        std::string("diam tellus a enim.Nam in fringilla turpis.\n") +
        std::string("Vestibulum pellentesque felis nec ex consectetur,\n") +
        std::string("non pulvinar lacus facilisis.Morbi lobortis metus\n") +
        std::string("ut tincidunt ullamcorper.\n") +
        std::string("\n");
    const std::string conversation2 = 
        std::string("1970-01-01 01:00:00\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n") +
        std::string("\n") +
        std::string("1970-01-01 01:00:00\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n") +
        std::string("\n") +
        std::string("                               1970-01-01 01:00:00\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string(" zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\n") +
        std::string("\n") +
        std::string("                               1970-01-01 01:00:00\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n") +
        std::string("\n");
    const auto& expected = std::vector<std::string>{conversation1, conversation2};
    const auto& actual = mOutputStreamMock->mOutput;
    EXPECT_EQ(actual, expected);
}

TEST_F(TTChatTest, UnhappyPathReceivedChunkMessagesNoMatch1) {
    // Expected strings
    const std::string longMessageChunk1(2048, 'x');
    // Expected calls
    EXPECT_CALL(*mPrimaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    const std::vector<TTChatMessage> messagesToBeReceived = {
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::SENDER_CHUNK, {}, longMessageChunk1),
        TTChatMessage(TTChatMessageType::RECEIVER_CHUNK, {}, longMessageChunk1)
    };
    const size_t receiveDelayTicks = 100;
    const auto receiveDelay = std::chrono::milliseconds{receiveDelayTicks};
    const auto sendDelay = std::chrono::milliseconds{0};
    const size_t minNumOfSentMessages = ((messagesToBeReceived.size() - 1) * receiveDelayTicks) / HEARTBEAT_TIMEOUT_MS;
    const size_t minNumOfReceivedMessages = messagesToBeReceived.size();
    {
        InSequence _;
        for (const auto &message : messagesToBeReceived) {
            EXPECT_CALL(*mPrimaryMessageQueueMock, receive)
                .Times(1)
                .WillOnce(DoAll(std::bind(&TTChatTest::SetArgPointerInReceiveMessage, this, _1, message, receiveDelay), Return(true)));
        }
    }
    EXPECT_CALL(*mSecondaryMessageQueueMock, send)
        .Times(AtLeast(minNumOfSentMessages))
        .WillRepeatedly(DoAll(std::bind(&TTChatTest::GetArgPointerInSendMessage, this, _1, sendDelay), Return(true)));
    // Run
    RestartApplication();
    VerifyApplicationTimeout(std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS * (minNumOfSentMessages + 1)});
    // Verify
    EXPECT_GE(mStoppedStatusOnReceive.size(), minNumOfReceivedMessages);
    for (size_t i = 0; i < minNumOfReceivedMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnReceive[i]) << "At some point application was stopped while receiving message!";
    }
    EXPECT_GT(mStoppedStatusOnSend.size(), minNumOfSentMessages);
    for (size_t i = 0; i < minNumOfSentMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnSend[i]) << "At some point application was stopped while sending message!";
    }
    for (const auto& sendMessage : mSendMessages) {
        EXPECT_EQ(sendMessage.getType(), TTChatMessageType::HEARTBEAT);
    }
}

TEST_F(TTChatTest, UnhappyPathReceivedChunkMessagesNoMatch2) {
    // Expected strings
    const std::string longMessageChunk1(2048, 'x');
    // Expected calls
    EXPECT_CALL(*mPrimaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    const std::vector<TTChatMessage> messagesToBeReceived = {
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::RECEIVER_CHUNK, {}, longMessageChunk1),
        TTChatMessage(TTChatMessageType::SENDER_CHUNK, {}, longMessageChunk1)
    };
    const size_t receiveDelayTicks = 100;
    const auto receiveDelay = std::chrono::milliseconds{receiveDelayTicks};
    const auto sendDelay = std::chrono::milliseconds{0};
    const size_t minNumOfSentMessages = ((messagesToBeReceived.size() - 1) * receiveDelayTicks) / HEARTBEAT_TIMEOUT_MS;
    const size_t minNumOfReceivedMessages = messagesToBeReceived.size();
    {
        InSequence _;
        for (const auto &message : messagesToBeReceived) {
            EXPECT_CALL(*mPrimaryMessageQueueMock, receive)
                .Times(1)
                .WillOnce(DoAll(std::bind(&TTChatTest::SetArgPointerInReceiveMessage, this, _1, message, receiveDelay), Return(true)));
        }
    }
    EXPECT_CALL(*mSecondaryMessageQueueMock, send)
        .Times(AtLeast(minNumOfSentMessages))
        .WillRepeatedly(DoAll(std::bind(&TTChatTest::GetArgPointerInSendMessage, this, _1, sendDelay), Return(true)));
    // Run
    RestartApplication();
    VerifyApplicationTimeout(std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS * (minNumOfSentMessages + 1)});
    // Verify
    EXPECT_GE(mStoppedStatusOnReceive.size(), minNumOfReceivedMessages);
    for (size_t i = 0; i < minNumOfReceivedMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnReceive[i]) << "At some point application was stopped while receiving message!";
    }
    EXPECT_GT(mStoppedStatusOnSend.size(), minNumOfSentMessages);
    for (size_t i = 0; i < minNumOfSentMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnSend[i]) << "At some point application was stopped while sending message!";
    }
    for (const auto& sendMessage : mSendMessages) {
        EXPECT_EQ(sendMessage.getType(), TTChatMessageType::HEARTBEAT);
    }
}

TEST_F(TTChatTest, UnhappyPathReceivedChunkMessagesNoMatch3) {
    // Expected strings
    const std::string longMessageChunk1(2048, 'x');
    // Expected calls
    EXPECT_CALL(*mPrimaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    const std::vector<TTChatMessage> messagesToBeReceived = {
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::RECEIVER_CHUNK, {}, longMessageChunk1),
        TTChatMessage(TTChatMessageType::SENDER, {}, longMessageChunk1)
    };
    const size_t receiveDelayTicks = 100;
    const auto receiveDelay = std::chrono::milliseconds{receiveDelayTicks};
    const auto sendDelay = std::chrono::milliseconds{0};
    const size_t minNumOfSentMessages = ((messagesToBeReceived.size() - 1) * receiveDelayTicks) / HEARTBEAT_TIMEOUT_MS;
    const size_t minNumOfReceivedMessages = messagesToBeReceived.size();
    {
        InSequence _;
        for (const auto &message : messagesToBeReceived) {
            EXPECT_CALL(*mPrimaryMessageQueueMock, receive)
                .Times(1)
                .WillOnce(DoAll(std::bind(&TTChatTest::SetArgPointerInReceiveMessage, this, _1, message, receiveDelay), Return(true)));
        }
    }
    EXPECT_CALL(*mSecondaryMessageQueueMock, send)
        .Times(AtLeast(minNumOfSentMessages))
        .WillRepeatedly(DoAll(std::bind(&TTChatTest::GetArgPointerInSendMessage, this, _1, sendDelay), Return(true)));
    // Run
    RestartApplication();
    VerifyApplicationTimeout(std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS * (minNumOfSentMessages + 1)});
    // Verify
    EXPECT_GE(mStoppedStatusOnReceive.size(), minNumOfReceivedMessages);
    for (size_t i = 0; i < minNumOfReceivedMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnReceive[i]) << "At some point application was stopped while receiving message!";
    }
    EXPECT_GT(mStoppedStatusOnSend.size(), minNumOfSentMessages);
    for (size_t i = 0; i < minNumOfSentMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnSend[i]) << "At some point application was stopped while sending message!";
    }
    for (const auto& sendMessage : mSendMessages) {
        EXPECT_EQ(sendMessage.getType(), TTChatMessageType::HEARTBEAT);
    }
}

TEST_F(TTChatTest, UnhappyPathReceivedChunkMessagesNoMatch4) {
    // Expected strings
    const std::string longMessageChunk1(2048, 'x');
    // Expected calls
    EXPECT_CALL(*mPrimaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    const std::vector<TTChatMessage> messagesToBeReceived = {
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::SENDER_CHUNK, {}, longMessageChunk1),
        TTChatMessage(TTChatMessageType::RECEIVER, {}, longMessageChunk1)
    };
    const size_t receiveDelayTicks = 100;
    const auto receiveDelay = std::chrono::milliseconds{receiveDelayTicks};
    const auto sendDelay = std::chrono::milliseconds{0};
    const size_t minNumOfSentMessages = ((messagesToBeReceived.size() - 1) * receiveDelayTicks) / HEARTBEAT_TIMEOUT_MS;
    const size_t minNumOfReceivedMessages = messagesToBeReceived.size();
    {
        InSequence _;
        for (const auto &message : messagesToBeReceived) {
            EXPECT_CALL(*mPrimaryMessageQueueMock, receive)
                .Times(1)
                .WillOnce(DoAll(std::bind(&TTChatTest::SetArgPointerInReceiveMessage, this, _1, message, receiveDelay), Return(true)));
        }
    }
    EXPECT_CALL(*mSecondaryMessageQueueMock, send)
        .Times(AtLeast(minNumOfSentMessages))
        .WillRepeatedly(DoAll(std::bind(&TTChatTest::GetArgPointerInSendMessage, this, _1, sendDelay), Return(true)));
    // Run
    RestartApplication();
    VerifyApplicationTimeout(std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS * (minNumOfSentMessages + 1)});
    // Verify
    EXPECT_GE(mStoppedStatusOnReceive.size(), minNumOfReceivedMessages);
    for (size_t i = 0; i < minNumOfReceivedMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnReceive[i]) << "At some point application was stopped while receiving message!";
    }
    EXPECT_GT(mStoppedStatusOnSend.size(), minNumOfSentMessages);
    for (size_t i = 0; i < minNumOfSentMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnSend[i]) << "At some point application was stopped while sending message!";
    }
    for (const auto& sendMessage : mSendMessages) {
        EXPECT_EQ(sendMessage.getType(), TTChatMessageType::HEARTBEAT);
    }
}

TEST_F(TTChatTest, UnhappyPathReceivedChunkMessagesNoMatch5) {
    // Expected strings
    const std::string longMessageChunk1(2048, 'x');
    // Expected calls
    EXPECT_CALL(*mPrimaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    const std::vector<TTChatMessage> messagesToBeReceived = {
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::SENDER_CHUNK, {}, longMessageChunk1),
        TTChatMessage(TTChatMessageType::CLEAR)
    };
    const size_t receiveDelayTicks = 100;
    const auto receiveDelay = std::chrono::milliseconds{receiveDelayTicks};
    const auto sendDelay = std::chrono::milliseconds{0};
    const size_t minNumOfSentMessages = ((messagesToBeReceived.size() - 1) * receiveDelayTicks) / HEARTBEAT_TIMEOUT_MS;
    const size_t minNumOfReceivedMessages = messagesToBeReceived.size();
    {
        InSequence _;
        for (const auto &message : messagesToBeReceived) {
            EXPECT_CALL(*mPrimaryMessageQueueMock, receive)
                .Times(1)
                .WillOnce(DoAll(std::bind(&TTChatTest::SetArgPointerInReceiveMessage, this, _1, message, receiveDelay), Return(true)));
        }
    }
    EXPECT_CALL(*mSecondaryMessageQueueMock, send)
        .Times(AtLeast(minNumOfSentMessages))
        .WillRepeatedly(DoAll(std::bind(&TTChatTest::GetArgPointerInSendMessage, this, _1, sendDelay), Return(true)));
    // Run
    RestartApplication();
    VerifyApplicationTimeout(std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS * (minNumOfSentMessages + 1)});
    // Verify
    EXPECT_GE(mStoppedStatusOnReceive.size(), minNumOfReceivedMessages);
    for (size_t i = 0; i < minNumOfReceivedMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnReceive[i]) << "At some point application was stopped while receiving message!";
    }
    EXPECT_GT(mStoppedStatusOnSend.size(), minNumOfSentMessages);
    for (size_t i = 0; i < minNumOfSentMessages; ++i) {
        EXPECT_FALSE(mStoppedStatusOnSend[i]) << "At some point application was stopped while sending message!";
    }
    for (const auto& sendMessage : mSendMessages) {
        EXPECT_EQ(sendMessage.getType(), TTChatMessageType::HEARTBEAT);
    }
}
