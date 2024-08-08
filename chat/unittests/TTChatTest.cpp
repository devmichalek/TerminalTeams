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
#include <span>

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
        mStoppedStatusOnReceive.emplace_back(mChat->stopped());
    }

    void GetArgPointerInSendMessage(const char* src, std::chrono::milliseconds timeout) {
        std::this_thread::sleep_for(timeout);
        TTChatMessage dst;
        std::memcpy(&dst, src, sizeof(dst));
        mSendMessages.push_back(dst);
        mStoppedStatusOnSend.emplace_back(mChat->stopped());
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
        EXPECT_CALL(*mSettingsMock, getTerminalWidth)
            .Times(1)
            .WillOnce(Return(TERMINAL_WIDTH));
        EXPECT_CALL(*mSettingsMock, getTerminalHeight)
            .Times(1)
            .WillOnce(Return(TERMINAL_HEIGHT));
        EXPECT_CALL(*mSettingsMock, getPrimaryMessageQueue)
            .Times(1)
            .WillOnce(Return(mPrimaryMessageQueueMock));
        EXPECT_CALL(*mSettingsMock, getSecondaryMessageQueue)
            .Times(1)
            .WillOnce(Return(mSecondaryMessageQueueMock));
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
        mApplicationCv.notify_one();
    }

    void RestartApplication() {
        mChat = std::make_unique<TTChat>(*mSettingsMock, *mOutputStreamMock);
        EXPECT_FALSE(mChat->stopped());
        mApplicationThread = std::thread{&TTChatTest::StartApplication, this};
    }

    void RestartApplicationNoCheck() {
        mChat = std::make_unique<TTChat>(*mSettingsMock, *mOutputStreamMock);
        mApplicationThread = std::thread{&TTChatTest::StartApplication, this};
    }

    void VerifyApplicationTimeout(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mApplicationMutex);
        const bool predicate = mApplicationCv.wait_for(lock, timeout, [this]() {
            return mChat->stopped();
        });
        EXPECT_TRUE(predicate); // Check for application timeout
        EXPECT_TRUE(mChat->stopped());
        mApplicationThread.join();
    }

    TTChatMessage CreateHeartbeatMessage() {
        TTChatMessage message;
        message.setType(TTChatMessageType::HEARTBEAT);
        return message;
    }

    TTChatMessage CreateGoodbyeMessage() {
        TTChatMessage message;
        message.setType(TTChatMessageType::GOODBYE);
        return message;
    }

    TTChatMessage CreateUnknownMessage() {
        TTChatMessage message;
        message.setType(static_cast<TTChatMessageType>(0xFF));
        return message;
    }

    TTChatMessage CreateClearMessage() {
        TTChatMessage message;
        message.setType(TTChatMessageType::CLEAR);
        return message;
    }

    TTChatMessage CreateSenderMessage(const std::string_view& data, TTChatTimestamp timestamp = {}) {
        TTChatMessage message;
        message.setType(TTChatMessageType::SENDER);
        message.setTimestamp(timestamp);
        message.setData(data);
        return message;
    }

    TTChatMessage CreateReceiverMessage(const std::string_view& data, TTChatTimestamp timestamp = {}) {
        TTChatMessage message;
        message.setType(TTChatMessageType::RECEIVER);
        message.setTimestamp(timestamp);
        message.setData(data);
        return message;
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
    const auto heartbeat = CreateHeartbeatMessage();
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
    const auto heartbeat = CreateHeartbeatMessage();
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
    const auto heartbeat = CreateHeartbeatMessage();
    const size_t minNumOfReceivedMessages = 5;
    const size_t minNumOfSentMessages = 5;
    const auto receiveDelay = std::chrono::milliseconds{40};
    const auto sendDelay = std::chrono::milliseconds{10};
    EXPECT_CALL(*mPrimaryMessageQueueMock, receive)
        .Times(AtLeast(minNumOfReceivedMessages))
        .WillRepeatedly(DoAll(std::bind(&TTChatTest::SetArgPointerInReceiveMessage, this, _1, heartbeat, receiveDelay), Return(true)));
    EXPECT_CALL(*mSecondaryMessageQueueMock, send)
        .Times(AtLeast(minNumOfSentMessages))
        .WillRepeatedly(DoAll(std::bind(&TTChatTest::GetArgPointerInSendMessage, this, _1, sendDelay), Return(true)));
    // Run
    RestartApplication();
    std::this_thread::sleep_for(std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS * (minNumOfSentMessages + 1)});
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
    const auto heartbeat = CreateHeartbeatMessage();
    const auto unknown = CreateUnknownMessage();
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
    const auto heartbeat = CreateHeartbeatMessage();
    const auto goodbye = CreateGoodbyeMessage();
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
        CreateHeartbeatMessage(),
        CreateClearMessage(),
        CreateSenderMessage("  \t  Hello, I'm having some additional whitespaces at the beginning"),
        CreateReceiverMessage("Hi, I've got additional whitespaces at the end  \t  "),
        CreateReceiverMessage("\t  \t   \t"),
        CreateGoodbyeMessage()
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

// TEST_F(TTChatTest, HappyPathReceivedMessagesDuplicatedWhitespaceCharacters) {

// }

// TEST_F(TTChatTest, HappyPathReceivedMessagesOnlyWhitespaceCharacters) {

// }

// TEST_F(TTChatTest, HappyPathReceivedMessagesTooManyWordsToFitInOneLine) {

// }

// TEST_F(TTChatTest, HappyPathReceivedMessagesMix) {

// }
