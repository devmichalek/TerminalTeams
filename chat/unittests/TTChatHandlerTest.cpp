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

    TTChatMessage CreateUnknownMessage() {
        TTChatMessage message;
        message.setType(static_cast<TTChatMessageType>(0xFF));
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
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::GOODBYE)
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
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::HEARTBEAT)
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
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::GOODBYE)
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
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(static_cast<TTChatMessageType>(0xFF))
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
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::GOODBYE)
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
    const auto messageToBeReceived = TTChatMessage(TTChatMessageType::HEARTBEAT);
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
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::GOODBYE)
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
    const auto messageToBeReceived = TTChatMessage(TTChatMessageType::HEARTBEAT);
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

TEST_F(TTChatHandlerTest, HappyAndUnhappyPathCreateSendAndReceive) {
    // Expected sent messages, entries
    std::vector<TTChatMessage> expectedSentMessages = {
        TTChatMessage(TTChatMessageType::HEARTBEAT),
        TTChatMessage(TTChatMessageType::SENDER, {}, "Hello Simon!"),
        TTChatMessage(TTChatMessageType::RECEIVER, {}, "Hi Tommy!"),
        TTChatMessage(TTChatMessageType::CLEAR),
        TTChatMessage(TTChatMessageType::SENDER, {}, "Hello from the other side!"),
        TTChatMessage(TTChatMessageType::RECEIVER, {}, "Welcome \"other side\"!"),
        TTChatMessage(TTChatMessageType::SENDER, {}, "Are you ok?"),
        TTChatMessage(TTChatMessageType::RECEIVER, {}, "Yes, thanks for asking"),
        TTChatMessage(TTChatMessageType::CLEAR),
        TTChatMessage(TTChatMessageType::SENDER, {}, "Hello Simon!"),
        TTChatMessage(TTChatMessageType::RECEIVER, {}, "Hi Tommy!"),
        TTChatMessage(TTChatMessageType::RECEIVER, {}, "How are you?"),
        TTChatMessage(TTChatMessageType::RECEIVER, {}, ""),
        TTChatMessage(TTChatMessageType::GOODBYE)
    };
    const TTChatEntries expectedEntries0 = {
        TTChatEntry{TTChatMessageType::SENDER, {}, "Hello Simon!"},
        TTChatEntry{TTChatMessageType::RECEIVER, {}, "Hi Tommy!"},
        TTChatEntry{TTChatMessageType::RECEIVER, {}, "How are you?"},
        TTChatEntry{TTChatMessageType::RECEIVER, {}, ""},
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
    const auto messageToBeReceived = TTChatMessage(TTChatMessageType::HEARTBEAT);
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
    EXPECT_TRUE(mChatHandler->receive(0, "", {}));
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

TEST_F(TTChatHandlerTest, HappyPathCreateSendAndReceiveHugeMessages) {
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
    // Expected sent messages, entries
    std::vector<TTChatMessage> expectedSentMessages;
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::HEARTBEAT));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::RECEIVER_CHUNK, {}, longMessage1Chunk1));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::RECEIVER, {}, longMessage1Chunk2));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::SENDER_CHUNK, {}, longMessage2Chunk1));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::SENDER, {}, longMessage2Chunk2));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::CLEAR));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::SENDER_CHUNK, {}, longMessage3Chunk1));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::SENDER_CHUNK, {}, longMessage3Chunk2));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::SENDER_CHUNK, {}, longMessage3Chunk3));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::SENDER_CHUNK, {}, longMessage3Chunk4));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::SENDER, {}, longMessage3Chunk5));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::SENDER, {}, longMessage4Chunk1));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::RECEIVER_CHUNK, {}, longMessage5Chunk1));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::RECEIVER, {}, longMessage5Chunk2));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::SENDER, {}, longMessage6Chunk1));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::CLEAR));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::RECEIVER_CHUNK, {}, longMessage1Chunk1));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::RECEIVER, {}, longMessage1Chunk2));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::SENDER_CHUNK, {}, longMessage2Chunk1));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::SENDER, {}, longMessage2Chunk2));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::RECEIVER_CHUNK, {}, longMessage3Chunk1));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::RECEIVER_CHUNK, {}, longMessage3Chunk2));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::RECEIVER_CHUNK, {}, longMessage3Chunk3));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::RECEIVER_CHUNK, {}, longMessage3Chunk4));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::RECEIVER, {}, longMessage3Chunk5));
    expectedSentMessages.push_back(TTChatMessage(TTChatMessageType::GOODBYE));
    const TTChatEntries expectedEntries0 = {
        TTChatEntry{TTChatMessageType::RECEIVER, {}, longMessage1},
        TTChatEntry{TTChatMessageType::SENDER, {}, longMessage2},
        TTChatEntry{TTChatMessageType::RECEIVER, {}, longMessage3}
    };
    const TTChatEntries expectedEntries1 = {
        TTChatEntry{TTChatMessageType::SENDER, {}, longMessage3},
        TTChatEntry{TTChatMessageType::SENDER, {}, longMessage4},
        TTChatEntry{TTChatMessageType::RECEIVER, {}, longMessage5},
        TTChatEntry{TTChatMessageType::SENDER, {}, longMessage6},
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
    const auto messageToBeReceived = TTChatMessage(TTChatMessageType::HEARTBEAT);
    EXPECT_CALL(*mSecondaryMessageQueueMock, receive)
        .Times(AtLeast(1))
        .WillRepeatedly(DoAll(std::bind(&TTChatHandlerTest::ProvideReceivedMessage, this, _1, messageToBeReceived, receiveDelay), Return(true)));
    EXPECT_CALL(*mPrimaryMessageQueueMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(DoAll(std::bind(&TTChatHandlerTest::RetrieveSentMessage, this, _1, sendDelay), Return(true)));
    // Verify
    EXPECT_TRUE(StartHandler(std::chrono::milliseconds{std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS + 100}}));
    // First chat
    EXPECT_TRUE(mChatHandler->create(0));
    EXPECT_EQ(mChatHandler->size(), 1);
    EXPECT_TRUE(mChatHandler->select(0));
    EXPECT_TRUE(mChatHandler->receive(0, longMessage1, {}));
    EXPECT_TRUE(mChatHandler->send(0, longMessage2, {}));
    EXPECT_NE(mChatHandler->current(), std::nullopt);
    EXPECT_EQ(mChatHandler->current().value(), 0);
    // Second chat
    EXPECT_TRUE(mChatHandler->create(1));
    EXPECT_TRUE(mChatHandler->select(1));
    EXPECT_TRUE(mChatHandler->send(1, longMessage3, {}));
    EXPECT_TRUE(mChatHandler->send(1, longMessage4, {}));
    EXPECT_TRUE(mChatHandler->receive(1, longMessage5, {}));
    EXPECT_TRUE(mChatHandler->send(1, longMessage6, {}));
    EXPECT_NE(mChatHandler->current(), std::nullopt);
    EXPECT_EQ(mChatHandler->current().value(), 1);
    // First chat continuation
    EXPECT_TRUE(mChatHandler->select(0));
    EXPECT_TRUE(mChatHandler->receive(0, longMessage3, {}));
    EXPECT_NE(mChatHandler->current(), std::nullopt);
    EXPECT_EQ(mChatHandler->current().value(), 0);
    // Check stopped status
    std::this_thread::sleep_for(std::chrono::milliseconds{HEARTBEAT_TIMEOUT_MS});
    EXPECT_FALSE(mChatHandler->stopped());
    EXPECT_TRUE(StopHandler(std::chrono::milliseconds{100}));
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
