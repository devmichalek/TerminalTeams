#include "TTChatHandler.hpp"
#include "TTChatSettingsMock.hpp"
#include "TTUtilsMessageQueueMock.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>
#include <functional>
#include <span>

using ::testing::Test;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::InSequence;
using ::testing::_;
using ::testing::AtLeast;
using namespace std::placeholders;

class TTChatHandlerTest : public Test {
public:
    void RetrieveSentMessage(const char* message, std::chrono::milliseconds timeout) {
        std::this_thread::sleep_for(timeout);
        TTChatMessage sentMessage;
        std::memcpy(&sentMessage, message, sizeof(sentMessage));
        mSentMessages.push_back(sentMessage);
        mStoppedStatusOnSend.emplace_back(mChatHandler->stopped());
    }

    void ProvideReceivedMessage(char* dst, const TTChatMessage& src, std::chrono::milliseconds timeout) {
        std::this_thread::sleep_for(timeout);
        std::memcpy(dst, &src, sizeof(src));
        mStoppedStatusOnReceive.emplace_back(mChatHandler->stopped());
    }

protected:
    TTChatHandlerTest() {
        mSettingsMock = std::make_shared<TTChatSettingsMock>();
        mPrimaryMessageQueueMock = std::make_shared<TTUtilsMessageQueueMock>();
        mSecondaryMessageQueueMock = std::make_shared<TTUtilsMessageQueueMock>();
    }
    ~TTChatHandlerTest() {

    }

    bool IsFirstEqualTo(const std::span<TTChatMessage>& lhs, const TTChatMessage& rhs) const {
        if (lhs.empty()) {
            return false;
        }
        return lhs.front() == rhs;
    }

    bool IsLastEqualTo(const std::span<TTChatMessage>& lhs, const TTChatMessage& rhs) const {
        if (lhs.empty()) {
            return false;
        }
        return lhs.back() == rhs;
    }

    bool IsEachEqualTo(const std::span<TTChatMessage>& lhs, const TTChatMessage& rhs) const {
        if (lhs.empty()) {
            return false;
        }
        if (std::adjacent_find(lhs.begin(), lhs.end(), std::not_equal_to<>()) == lhs.end()) {
            return lhs.front() == rhs;
        }
        return false;
    }

    bool IsAtLeastOneEqualTo(const std::span<TTChatMessage>& lhs, const TTChatMessage& rhs) const {
        if (lhs.empty()) {
            return false;
        }
        return std::find(lhs.begin(), lhs.end(), rhs) != lhs.end();
    }

    bool IsOrderEqualTo(const std::span<TTChatMessage>& lhs, const std::span<TTChatMessage>& rhs) const {
        if (rhs.empty()) {
            return lhs.empty();
        }
        auto result = lhs.begin();
        for (const auto& i : rhs) {
            if (result == lhs.end()) {
                break;
            }
            result = std::find(result, lhs.end(), i);
        }
        return result != lhs.end();
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

    // Called after constructor, before each test
    virtual void SetUp() override {
        EXPECT_CALL(*mSettingsMock, getPrimaryMessageQueue)
            .Times(1)
            .WillOnce(Return(mPrimaryMessageQueueMock));
        EXPECT_CALL(*mSettingsMock, getSecondaryMessageQueue)
            .Times(1)
            .WillOnce(Return(mSecondaryMessageQueueMock));
    }
    // Called before destructor, after each test
    virtual void TearDown() override {
        mSentMessages.clear();
        mStoppedStatusOnSend.clear();
        mStoppedStatusOnReceive.clear();
    }

    bool StartHandler(std::chrono::milliseconds timeout) {
        mChatHandler.reset(new TTChatHandler(*mSettingsMock));
        std::this_thread::sleep_for(timeout);
        return !mChatHandler->stopped();
    }

    bool StopHandler(std::chrono::milliseconds timeout) {
        if (mChatHandler) {
            mChatHandler->stop();
            std::this_thread::sleep_for(timeout);
            return mChatHandler->stopped();
        }
        return false;
    }

    std::shared_ptr<TTChatSettingsMock> mSettingsMock;
    std::shared_ptr<TTUtilsMessageQueueMock> mPrimaryMessageQueueMock;
    std::shared_ptr<TTUtilsMessageQueueMock> mSecondaryMessageQueueMock;
    std::unique_ptr<TTChatHandler> mChatHandler;
    std::vector<TTChatMessage> mSentMessages;
    std::vector<bool> mStoppedStatusOnSend;
    std::vector<bool> mStoppedStatusOnReceive;
    constexpr static long HEARTBEAT_TIMEOUT_MS = 500; // 0.5s
};

TEST_F(TTChatHandlerTest, FailedToCreatePrimaryMessageQueue) {
    EXPECT_CALL(*mPrimaryMessageQueueMock, create)
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_THROW(StartHandler(std::chrono::milliseconds{0}), std::runtime_error);
    EXPECT_FALSE(StopHandler(std::chrono::milliseconds{0}));
}

TEST_F(TTChatHandlerTest, FailedToCreateSecondaryMessageQueue) {
    EXPECT_CALL(*mPrimaryMessageQueueMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, create)
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_THROW(StartHandler(std::chrono::milliseconds{0}), std::runtime_error);
    EXPECT_FALSE(StopHandler(std::chrono::milliseconds{0}));
}

TEST_F(TTChatHandlerTest, FailedToRunPrimaryMessageQueueNotAlive) {
    EXPECT_CALL(*mPrimaryMessageQueueMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));
    EXPECT_FALSE(StartHandler(std::chrono::milliseconds{100}));
    EXPECT_TRUE(StopHandler(std::chrono::milliseconds{0}));
}

TEST_F(TTChatHandlerTest, FailedToRunSecondaryMessageQueueNotAlive) {
    EXPECT_CALL(*mPrimaryMessageQueueMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));
    EXPECT_FALSE(StartHandler(std::chrono::milliseconds{100}));
    EXPECT_TRUE(StopHandler(std::chrono::milliseconds{0}));
}

TEST_F(TTChatHandlerTest, FailedToReceiveAfterManyReceivedHeartbeats) {
    // Expected sent messages
    const std::vector<TTChatMessage> expectedSentMessages = {
        CreateHeartbeatMessage(),
        CreateGoodbyeMessage()
    };
    // Expected calls
    EXPECT_CALL(*mPrimaryMessageQueueMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    const auto receiveDelay = std::chrono::milliseconds{400};
    const auto sendDelay = std::chrono::milliseconds{0};
    const std::vector<TTChatMessage> messagesToBeReceived = {
        CreateHeartbeatMessage(),
        CreateHeartbeatMessage(),
        CreateHeartbeatMessage(),
        CreateHeartbeatMessage()
    };
    const size_t minNumOfSentMessages = 1;
    {
        InSequence _;
        for (const auto &message : messagesToBeReceived) {
            EXPECT_CALL(*mSecondaryMessageQueueMock, receive)
                .Times(1)
                .WillOnce(DoAll(std::bind(&TTChatHandlerTest::ProvideReceivedMessage, this, _1, message, receiveDelay), Return(true)));
        }
        EXPECT_CALL(*mSecondaryMessageQueueMock, receive)
            .Times(1)
            .WillOnce(Return(false));
    }
    EXPECT_CALL(*mPrimaryMessageQueueMock, send)
        .Times(AtLeast(minNumOfSentMessages))
        .WillRepeatedly(DoAll(std::bind(&TTChatHandlerTest::RetrieveSentMessage, this, _1, sendDelay), Return(true)));
    // Verify
    EXPECT_TRUE(StartHandler(std::chrono::milliseconds{std::chrono::milliseconds{100}}));
    std::this_thread::sleep_for(std::chrono::milliseconds{2200});
    EXPECT_EQ(mChatHandler->size(), 0);
    EXPECT_EQ(mChatHandler->current(), std::nullopt);
    EXPECT_TRUE(mChatHandler->stopped());
    for (const auto status : mStoppedStatusOnReceive) {
        EXPECT_FALSE(status) << "At some point application was stopped while receiving message!";
    }
    EXPECT_GT(mStoppedStatusOnSend.size(), 2);
    mStoppedStatusOnSend.pop_back();
    mStoppedStatusOnSend.pop_back();
    for (const auto status : mStoppedStatusOnSend) {
        EXPECT_FALSE(status) << "At some point application was stopped while receiving message!";
    }
    EXPECT_TRUE(IsFirstEqualTo(mSentMessages, expectedSentMessages.front()));
    EXPECT_TRUE(IsLastEqualTo(mSentMessages, expectedSentMessages.back()));
}

TEST_F(TTChatHandlerTest, FailedToReceiveAfterManyReceivedHeartbeatsThenUnknown) {
    // Expected sent messages
    const std::vector<TTChatMessage> expectedSentMessages = {
        CreateHeartbeatMessage(),
        CreateGoodbyeMessage()
    };
    // Expected calls
    EXPECT_CALL(*mPrimaryMessageQueueMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    const auto receiveDelay = std::chrono::milliseconds{400};
    const auto sendDelay = std::chrono::milliseconds{0};
    const std::vector<TTChatMessage> messagesToBeReceived = {
        CreateHeartbeatMessage(),
        CreateHeartbeatMessage(),
        CreateHeartbeatMessage(),
        CreateHeartbeatMessage(),
        CreateUnknownMessage()
    };
    const size_t minNumOfSentMessages = 1;
    {
        InSequence _;
        for (const auto &message : messagesToBeReceived) {
            EXPECT_CALL(*mSecondaryMessageQueueMock, receive)
                .Times(1)
                .WillOnce(DoAll(std::bind(&TTChatHandlerTest::ProvideReceivedMessage, this, _1, message, receiveDelay), Return(true)));
        }
    }
    EXPECT_CALL(*mPrimaryMessageQueueMock, send)
        .Times(AtLeast(minNumOfSentMessages))
        .WillRepeatedly(DoAll(std::bind(&TTChatHandlerTest::RetrieveSentMessage, this, _1, sendDelay), Return(true)));
    // Verify
    EXPECT_TRUE(StartHandler(std::chrono::milliseconds{std::chrono::milliseconds{100}}));
    std::this_thread::sleep_for(std::chrono::milliseconds{2200});
    EXPECT_EQ(mChatHandler->size(), 0);
    EXPECT_EQ(mChatHandler->current(), std::nullopt);
    EXPECT_TRUE(mChatHandler->stopped());
    for (const auto status : mStoppedStatusOnReceive) {
        EXPECT_FALSE(status) << "At some point application was stopped while receiving message!";
    }
    EXPECT_GT(mStoppedStatusOnSend.size(), 2);
    mStoppedStatusOnSend.pop_back();
    mStoppedStatusOnSend.pop_back();
    for (const auto status : mStoppedStatusOnSend) {
        EXPECT_FALSE(status) << "At some point application was stopped while receiving message!";
    }
    EXPECT_TRUE(IsFirstEqualTo(mSentMessages, expectedSentMessages.front()));
    EXPECT_TRUE(IsLastEqualTo(mSentMessages, expectedSentMessages.back()));
}

TEST_F(TTChatHandlerTest, FailedToSendAfterManySendHeartbeats) {
    // Expected sent messages
    const std::vector<TTChatMessage> expectedSentMessages = {
        CreateHeartbeatMessage(),
        CreateGoodbyeMessage()
    };
    // Expected calls
    EXPECT_CALL(*mPrimaryMessageQueueMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    const auto receiveDelay = std::chrono::milliseconds{200};
    const auto sendDelay = std::chrono::milliseconds{0};
    const auto messageToBeReceived = CreateHeartbeatMessage();
    const size_t minNumOfSentMessages = 3;
    EXPECT_CALL(*mSecondaryMessageQueueMock, receive)
        .Times(AtLeast(1))
        .WillRepeatedly(DoAll(std::bind(&TTChatHandlerTest::ProvideReceivedMessage, this, _1, messageToBeReceived, receiveDelay), Return(true)));
    {
        InSequence _;
        EXPECT_CALL(*mPrimaryMessageQueueMock, send)
            .Times(minNumOfSentMessages)
            .WillRepeatedly(DoAll(std::bind(&TTChatHandlerTest::RetrieveSentMessage, this, _1, sendDelay), Return(true)));
        EXPECT_CALL(*mPrimaryMessageQueueMock, send)
            .Times(2)
            .WillRepeatedly(DoAll(std::bind(&TTChatHandlerTest::RetrieveSentMessage, this, _1, sendDelay), Return(false)));
    }
    // Verify
    EXPECT_TRUE(StartHandler(std::chrono::milliseconds{std::chrono::milliseconds{100}}));
    std::this_thread::sleep_for(std::chrono::milliseconds{2500});
    EXPECT_EQ(mChatHandler->size(), 0);
    EXPECT_EQ(mChatHandler->current(), std::nullopt);
    EXPECT_TRUE(mChatHandler->stopped());
    EXPECT_GT(mStoppedStatusOnReceive.size(), 1);
    mStoppedStatusOnReceive.pop_back();
    for (const auto status : mStoppedStatusOnReceive) {
        EXPECT_FALSE(status) << "At some point application was stopped while receiving message!";
    }
    EXPECT_GT(mStoppedStatusOnSend.size(), 2);
    mStoppedStatusOnSend.pop_back();
    mStoppedStatusOnSend.pop_back();
    for (const auto status : mStoppedStatusOnSend) {
        EXPECT_FALSE(status) << "At some point application was stopped while receiving message!";
    }
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end() - 1}, expectedSentMessages.front()));
    EXPECT_TRUE(IsLastEqualTo(mSentMessages, expectedSentMessages.back()));
}

TEST_F(TTChatHandlerTest, HappyPathAtLeastThreeHeartbeatsAndGoodbye) {
    // Expected sent messages
    const std::vector<TTChatMessage> expectedSentMessages = {
        CreateHeartbeatMessage(),
        CreateGoodbyeMessage()
    };
    // Expected calls
    EXPECT_CALL(*mPrimaryMessageQueueMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    const auto receiveDelay = std::chrono::milliseconds{20};
    const auto sendDelay = std::chrono::milliseconds{10};
    const auto messageToBeReceived = CreateHeartbeatMessage();
    EXPECT_CALL(*mSecondaryMessageQueueMock, receive)
        .Times(AtLeast(1))
        .WillRepeatedly(DoAll(std::bind(&TTChatHandlerTest::ProvideReceivedMessage, this, _1, messageToBeReceived, receiveDelay), Return(true)));
    EXPECT_CALL(*mPrimaryMessageQueueMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(DoAll(std::bind(&TTChatHandlerTest::RetrieveSentMessage, this, _1, sendDelay), Return(true)));
    // Verify
    EXPECT_TRUE(StartHandler(std::chrono::milliseconds{std::chrono::milliseconds{100}}));
    std::this_thread::sleep_for(std::chrono::milliseconds{2000});
    EXPECT_EQ(mChatHandler->size(), 0);
    EXPECT_EQ(mChatHandler->current(), std::nullopt);
    EXPECT_FALSE(mChatHandler->stopped());
    EXPECT_TRUE(StopHandler(std::chrono::milliseconds{100}));
    EXPECT_GT(mStoppedStatusOnReceive.size(), 1);
    mStoppedStatusOnReceive.pop_back();
    for (const auto status : mStoppedStatusOnReceive) {
        EXPECT_FALSE(status) << "At some point application was stopped while receiving message!";
    }
    EXPECT_GT(mStoppedStatusOnSend.size(), 2);
    mStoppedStatusOnSend.pop_back();
    mStoppedStatusOnSend.pop_back();
    for (const auto status : mStoppedStatusOnSend) {
        EXPECT_FALSE(status) << "At some point application was stopped while receiving message!";
    }
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end() - 1}, expectedSentMessages.front()));
    EXPECT_TRUE(IsLastEqualTo(mSentMessages, expectedSentMessages.back()));
}

TEST_F(TTChatHandlerTest, HappyPathCreateSendAndReceive) {
    // Expected sent messages, entries
    std::vector<TTChatMessage> expectedSentMessages = {
        CreateHeartbeatMessage(),
        CreateSenderMessage("Hello Simon!"),
        CreateReceiverMessage("Hi Tommy!"),
        CreateClearMessage(),
        CreateSenderMessage("Hello from the other side!"),
        CreateReceiverMessage("Welcome \"other side\"!"),
        CreateSenderMessage("Are you ok?"),
        CreateReceiverMessage("Yes, thanks for asking"),
        CreateClearMessage(),
        CreateSenderMessage("Hello Simon!"),
        CreateReceiverMessage("Hi Tommy!"),
        CreateReceiverMessage("How are you?"),
        CreateGoodbyeMessage()
    };
    const TTChatEntries expectedEntries0 = {
        TTChatEntry{TTChatMessageType::SENDER, {}, "Hello Simon!"},
        TTChatEntry{TTChatMessageType::RECEIVER, {}, "Hi Tommy!"},
        TTChatEntry{TTChatMessageType::RECEIVER, {}, "How are you?"}
    };
    const TTChatEntries expectedEntries1 = {
        TTChatEntry{TTChatMessageType::SENDER, {}, "Hello from the other side!"},
        TTChatEntry{TTChatMessageType::RECEIVER, {}, "Welcome \"other side\"!"},
        TTChatEntry{TTChatMessageType::SENDER, {}, "Are you ok?"},
        TTChatEntry{TTChatMessageType::RECEIVER, {}, "Yes, thanks for asking"},
    };
    // Expected calls
    EXPECT_CALL(*mPrimaryMessageQueueMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mPrimaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mSecondaryMessageQueueMock, alive)
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    const auto receiveDelay = std::chrono::milliseconds{20};
    const auto sendDelay = std::chrono::milliseconds{10};
    const auto messageToBeReceived = CreateHeartbeatMessage();
    EXPECT_CALL(*mSecondaryMessageQueueMock, receive)
        .Times(AtLeast(1))
        .WillRepeatedly(DoAll(std::bind(&TTChatHandlerTest::ProvideReceivedMessage, this, _1, messageToBeReceived, receiveDelay), Return(true)));
    EXPECT_CALL(*mPrimaryMessageQueueMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(DoAll(std::bind(&TTChatHandlerTest::RetrieveSentMessage, this, _1, sendDelay), Return(true)));
    // Verify
    EXPECT_TRUE(StartHandler(std::chrono::milliseconds{std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS + 100}}));
    // Create first contact
    EXPECT_FALSE(mChatHandler->create(1));
    EXPECT_TRUE(mChatHandler->create(0));
    EXPECT_FALSE(mChatHandler->create(0));
    // Attempt to send, receive or select non existing contact
    EXPECT_FALSE(mChatHandler->send(1, "Dummy", {}));
    EXPECT_FALSE(mChatHandler->receive(1, "Dummy", {}));
    EXPECT_FALSE(mChatHandler->select(1));
    // Check if currently selected contacts is non-existing and size is correct
    EXPECT_EQ(mChatHandler->current(), std::nullopt);
    EXPECT_EQ(mChatHandler->size(), 1);
    // Send and receive attempts
    EXPECT_FALSE(mChatHandler->send(0, "Dummy", {}));
    EXPECT_TRUE(mChatHandler->select(0));
    EXPECT_TRUE(mChatHandler->send(0, "Hello Simon!", {}));
    EXPECT_TRUE(mChatHandler->receive(0, "Hi Tommy!", {}));
    EXPECT_FALSE(mChatHandler->select(0));
    EXPECT_NE(mChatHandler->current(), std::nullopt);
    EXPECT_EQ(mChatHandler->current().value(), 0);
    // Create second contact
    EXPECT_TRUE(mChatHandler->create(1));
    EXPECT_FALSE(mChatHandler->create(1));
    // Select second contact and add chat
    EXPECT_TRUE(mChatHandler->select(1));
    EXPECT_TRUE(mChatHandler->send(1, "Hello from the other side!", {}));
    EXPECT_TRUE(mChatHandler->receive(1, "Welcome \"other side\"!", {}));
    // Receive on non selected contact
    EXPECT_TRUE(mChatHandler->receive(0, "How are you?", {}));
    EXPECT_FALSE(mChatHandler->send(0, "Dummy", {}));
    EXPECT_TRUE(mChatHandler->send(1, "Are you ok?", {}));
    EXPECT_TRUE(mChatHandler->receive(1, "Yes, thanks for asking", {}));
    // Select again the first contact and answer
    EXPECT_TRUE(mChatHandler->select(0));
    EXPECT_FALSE(mChatHandler->send(1, "Good, and you?", {}));
    // Some sleep and other checks
    std::this_thread::sleep_for(std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS});
    EXPECT_FALSE(mChatHandler->stopped());
    EXPECT_TRUE(StopHandler(std::chrono::milliseconds{100}));
    EXPECT_FALSE(mChatHandler->create(1));
    EXPECT_FALSE(mChatHandler->send(0, "Dummy", {}));
    EXPECT_FALSE(mChatHandler->receive(0, "Dummy", {}));
    EXPECT_FALSE(mChatHandler->select(0));
    EXPECT_NE(mChatHandler->current(), std::nullopt);
    EXPECT_EQ(mChatHandler->current().value(), 0);
    // Check entries
    EXPECT_NE(mChatHandler->get(0), std::nullopt);
    const auto actualEntries0 = mChatHandler->get(0).value();
    EXPECT_EQ(mChatHandler->get(0).value(), expectedEntries0);
    EXPECT_NE(mChatHandler->get(1), std::nullopt);
    const auto actualEntries1 = mChatHandler->get(1).value();
    EXPECT_EQ(mChatHandler->get(1).value(), expectedEntries1);
    // Check messages
    EXPECT_GT(mStoppedStatusOnReceive.size(), 1);
    mStoppedStatusOnReceive.pop_back();
    for (const auto status : mStoppedStatusOnReceive) {
        EXPECT_FALSE(status) << "At some point application was stopped while receiving message!";
    }
    EXPECT_GT(mStoppedStatusOnSend.size(), 2);
    mStoppedStatusOnSend.pop_back();
    mStoppedStatusOnSend.pop_back();
    for (const auto status : mStoppedStatusOnSend) {
        EXPECT_FALSE(status) << "At some point application was stopped while receiving message!";
    }
    EXPECT_TRUE(IsFirstEqualTo(mSentMessages, expectedSentMessages.front()));
    EXPECT_TRUE(IsOrderEqualTo(mSentMessages, {expectedSentMessages.begin() + 1, expectedSentMessages.end() - 1}));
    EXPECT_TRUE(IsLastEqualTo(mSentMessages, expectedSentMessages.back()));
}

// TEST_F(TTChatHandlerTest, HappyPathHugeMessages) {
    
// }

// TEST_F(TTChatHandlerTest, UnhappyPathUsageAfterStop) {
// }

// TEST_F(TTChatHandlerTest, UnhappyPathIdentityOutOfRange) {
// }
