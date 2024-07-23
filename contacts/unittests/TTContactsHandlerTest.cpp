#include "TTContactsHandler.hpp"
#include "TTContactsSettingsMock.hpp"
#include "TTUtilsSharedMemMock.hpp"
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

class TTContactsHandlerTest : public Test {
public:
    bool RetrieveSentMessageTrue(const void* message, long attempts, long timeoutMs) {
        TTContactsMessage sentMessage;
        std::memcpy(&sentMessage, message, sizeof(sentMessage));
        mSentMessages.push_back(sentMessage);
        return true;
    }

    bool RetrieveSentMessageFalse(const void* message, long attempts, long timeoutMs) {
        TTContactsMessage sentMessage;
        std::memcpy(&sentMessage, message, sizeof(sentMessage));
        mSentMessages.push_back(sentMessage);
        return false;
    }
protected:
    TTContactsHandlerTest() {
        mSettingsMock = std::make_shared<TTContactsSettingsMock>();
        mSharedMemMock = std::make_shared<TTUtilsSharedMemMock>();
    }
    ~TTContactsHandlerTest() {

    }

    // Called after constructor, before each test
    virtual void SetUp() override {
        EXPECT_CALL(*mSettingsMock, getSharedMemory)
            .Times(1)
            .WillOnce(Return(mSharedMemMock));
    }
    // Called before destructor, after each test
    virtual void TearDown() override {
        mSentMessages.clear();
        mExpectedMessages.clear();
        mExpectedEntries.clear();
    }

    bool IsFirstEqualTo(const std::span<TTContactsMessage>& lhs, const TTContactsMessage& rhs) const {
        return lhs.front() == rhs;
    }

    bool IsLastEqualTo(const std::span<TTContactsMessage>& lhs, const TTContactsMessage& rhs) const {
        return lhs.back() == rhs;
    }

    bool IsEachEqualTo(const std::span<TTContactsMessage>& lhs, const TTContactsMessage& rhs) const {
        if (std::adjacent_find(lhs.begin(), lhs.end(), std::not_equal_to<>()) == lhs.end()) {
            return lhs.front() == rhs;
        }
        return false;
    }

    bool IsAtLeastOneEqualTo(const std::span<TTContactsMessage>& lhs, const TTContactsMessage& rhs) const {
        return std::find(lhs.begin(), lhs.end(), rhs) != lhs.end();
    }

    bool IsOrderEqualTo(const std::span<TTContactsMessage>& lhs, const std::span<TTContactsMessage>& rhs) const {
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

    bool StartHandler(std::chrono::milliseconds timeout) {
        mContactsHandler.reset(new TTContactsHandler(*mSettingsMock));
        std::this_thread::sleep_for(timeout);
        return !mContactsHandler->stopped();
    }

    bool StopHandler() {
        if (mContactsHandler) {
            mContactsHandler->stop();
            std::this_thread::sleep_for(std::chrono::milliseconds(TTCONTACTS_HEARTBEAT_TIMEOUT_MS));
            return mContactsHandler->stopped();
        }
        return false;
    }

    void CreateMessage(TTContactsStatus status,
                       TTContactsState state = TTContactsState::ACTIVE,
                       size_t identity = 0,
                       const std::string& nickname = "") {
        TTContactsMessage result;
        result.setStatus(status);
        result.setState(state);
        result.setIdentity(identity);
        result.setNickname(nickname);
        mExpectedMessages.push_back(result);
    }

    void CreateEntry(const std::string& nickname,
                     const std::string& identity,
                     const std::string& ipAddressAndPort,
                     TTContactsState state,
                     size_t sentMessages,
                     size_t receivedMessages) {
        mExpectedEntries.emplace_back(nickname, identity, ipAddressAndPort);
        mExpectedEntries.back().state = state;
        mExpectedEntries.back().sentMessages = sentMessages;
        mExpectedEntries.back().receivedMessages = receivedMessages;
    }

    std::shared_ptr<TTContactsSettingsMock> mSettingsMock;
    std::shared_ptr<TTUtilsSharedMemMock> mSharedMemMock;
    std::unique_ptr<TTContactsHandler> mContactsHandler;
    std::vector<TTContactsMessage> mSentMessages;
    std::vector<TTContactsMessage> mExpectedMessages;
    std::vector<TTContactsHandlerEntry> mExpectedEntries;
};

TEST_F(TTContactsHandlerTest, FailedToInitSharedMemory) {
    EXPECT_CALL(*mSharedMemMock, create)
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_THROW(StartHandler(std::chrono::milliseconds{0}), std::runtime_error);
    EXPECT_FALSE(StopHandler());
}

TEST_F(TTContactsHandlerTest, FailedToEstablishConnection) {
    CreateMessage(TTContactsStatus::HEARTBEAT);
    EXPECT_CALL(*mSharedMemMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSharedMemMock, send(_, _, _))
        .WillOnce(std::bind(&TTContactsHandlerTest::RetrieveSentMessageFalse, this, _1, _2, _3));
    EXPECT_THROW(StartHandler(std::chrono::milliseconds{0}), std::runtime_error);
    EXPECT_EQ(mSentMessages.size(), 1);
    EXPECT_TRUE(IsFirstEqualTo(mSentMessages, mExpectedMessages.front()));
    EXPECT_FALSE(StopHandler());
}

TEST_F(TTContactsHandlerTest, SuccessAtLeastThreeHeartbeatsAndGoodbye) {
    CreateMessage(TTContactsStatus::HEARTBEAT);
    CreateMessage(TTContactsStatus::GOODBYE);
    const size_t expectedMinNumOfMessages = 3;
    EXPECT_CALL(*mSharedMemMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSharedMemMock, send)
        .Times(AtLeast(expectedMinNumOfMessages))
        .WillRepeatedly(std::bind(&TTContactsHandlerTest::RetrieveSentMessageTrue, this, _1, _2, _3));
    EXPECT_CALL(*mSharedMemMock, destroy)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(StartHandler(std::chrono::milliseconds{TTCONTACTS_HEARTBEAT_TIMEOUT_MS * expectedMinNumOfMessages}));
    EXPECT_TRUE(StopHandler());
    EXPECT_GT(mSentMessages.size(), expectedMinNumOfMessages);
    EXPECT_TRUE(IsLastEqualTo(mSentMessages, mExpectedMessages.back()));
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end() - 1}, mExpectedMessages.front()));
}

TEST_F(TTContactsHandlerTest, SuccessSelectionMachineState) {
    // Expected messages
    CreateMessage(TTContactsStatus::HEARTBEAT);
    CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 0, "A");
    CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_ACTIVE, 0, "A");
    CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 1, "B");
    CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 2, "C");
    CreateMessage(TTContactsStatus::GOODBYE);
    // Expected entries
    CreateEntry("A", "0feca842", "192.168.1.15", TTContactsState::SELECTED_ACTIVE, 0, 0);
    CreateEntry("B", "09dda800", "192.168.1.16", TTContactsState::ACTIVE, 0, 0);
    CreateEntry("C", "000ca777", "192.168.1.17", TTContactsState::ACTIVE, 0, 0);
    // Expected calls
    EXPECT_CALL(*mSharedMemMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSharedMemMock, send)
        .Times(AtLeast(mExpectedMessages.size()))
        .WillRepeatedly(std::bind(&TTContactsHandlerTest::RetrieveSentMessageTrue, this, _1, _2, _3));
    EXPECT_CALL(*mSharedMemMock, destroy)
        .Times(1)
        .WillOnce(Return(true));
    // Flow
    EXPECT_TRUE(StartHandler(std::chrono::milliseconds{TTCONTACTS_HEARTBEAT_TIMEOUT_MS}));
    // -> ACTIVE, ACTIVE -> SELECTED_ACTIVE, -> ACTIVE, -> ACTIVE
    EXPECT_TRUE(mContactsHandler->create(mExpectedEntries[0].nickname, mExpectedEntries[0].identity, mExpectedEntries[0].ipAddressAndPort));
    EXPECT_TRUE(mContactsHandler->select(0));
    EXPECT_TRUE(mContactsHandler->create(mExpectedEntries[1].nickname, mExpectedEntries[1].identity, mExpectedEntries[1].ipAddressAndPort));
    EXPECT_TRUE(mContactsHandler->create(mExpectedEntries[2].nickname, mExpectedEntries[2].identity, mExpectedEntries[2].ipAddressAndPort));
    std::this_thread::sleep_for(std::chrono::milliseconds{TTCONTACTS_HEARTBEAT_TIMEOUT_MS});
    EXPECT_TRUE(StopHandler());
    // Expected data
    EXPECT_GT(mSentMessages.size(), mExpectedMessages.size());
    EXPECT_TRUE(IsFirstEqualTo(mSentMessages, mExpectedMessages.front()));
    EXPECT_TRUE(IsLastEqualTo(mSentMessages, mExpectedMessages.back()));
    EXPECT_TRUE(IsOrderEqualTo({mSentMessages.begin(), mSentMessages.end()}, {mExpectedMessages.begin() + 1, mExpectedMessages.end() - 1}));
    for (size_t i = 0; i < mExpectedEntries.size(); ++i) {
        ASSERT_NE(mContactsHandler->get(i), std::nullopt);
        EXPECT_EQ(mContactsHandler->get(i).value(), mExpectedEntries[i]);
        ASSERT_NE(mContactsHandler->get(mExpectedEntries[i].identity), std::nullopt);
        EXPECT_EQ(mContactsHandler->get(mExpectedEntries[i].identity).value(), i);
    }
    ASSERT_NE(mContactsHandler->current(), std::nullopt);
    EXPECT_EQ(mContactsHandler->current().value(), 0);
    EXPECT_EQ(mContactsHandler->size(), mExpectedEntries.size());
}

TEST_F(TTContactsHandlerTest, SuccessSendAndReceiveMachineState) {
    // Expected messages
    CreateMessage(TTContactsStatus::HEARTBEAT);
    CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 0, "A");
    CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_ACTIVE, 0, "A");
    CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 1, "B");
    CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 2, "C");
    CreateMessage(TTContactsStatus::STATE, TTContactsState::INACTIVE, 1, "B");
    CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 0, "A");
    CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_INACTIVE, 1, "B");
    CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_PENDING_MSG_INACTIVE, 1, "B");
    CreateMessage(TTContactsStatus::STATE, TTContactsState::PENDING_MSG_INACTIVE, 1, "B");
    CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_ACTIVE, 2, "C");
    CreateMessage(TTContactsStatus::STATE, TTContactsState::ACTIVE, 2, "C");
    CreateMessage(TTContactsStatus::STATE, TTContactsState::SELECTED_ACTIVE, 0, "A");
    CreateMessage(TTContactsStatus::STATE, TTContactsState::UNREAD_MSG_ACTIVE, 2, "C");
    CreateMessage(TTContactsStatus::GOODBYE);
    // Expected entries
    CreateEntry("A", "0feca842", "192.168.1.15", TTContactsState::SELECTED_ACTIVE, 1, 0);
    CreateEntry("B", "09dda800", "192.168.1.16", TTContactsState::PENDING_MSG_INACTIVE, 2, 0);
    CreateEntry("C", "000ca777", "192.168.1.17", TTContactsState::UNREAD_MSG_ACTIVE, 0, 3);
    // Expected calls
    EXPECT_CALL(*mSharedMemMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSharedMemMock, send)
        .Times(AtLeast(mExpectedMessages.size()))
        .WillRepeatedly(std::bind(&TTContactsHandlerTest::RetrieveSentMessageTrue, this, _1, _2, _3));
    EXPECT_CALL(*mSharedMemMock, destroy)
        .Times(1)
        .WillOnce(Return(true));
    // Flow
    EXPECT_TRUE(StartHandler(std::chrono::milliseconds{TTCONTACTS_HEARTBEAT_TIMEOUT_MS}));
    // -> ACTIVE, ACTIVE -> SELECTED_ACTIVE, -> ACTIVE, -> ACTIVE
    EXPECT_TRUE(mContactsHandler->create(mExpectedEntries[0].nickname, mExpectedEntries[0].identity, mExpectedEntries[0].ipAddressAndPort));
    EXPECT_TRUE(mContactsHandler->select(0));
    EXPECT_TRUE(mContactsHandler->create(mExpectedEntries[1].nickname, mExpectedEntries[1].identity, mExpectedEntries[1].ipAddressAndPort));
    EXPECT_TRUE(mContactsHandler->create(mExpectedEntries[2].nickname, mExpectedEntries[2].identity, mExpectedEntries[2].ipAddressAndPort));
    // SELECTED_ACTIVE -> OK
    EXPECT_TRUE(mContactsHandler->send(0));
    // ACTIVE -> INACTIVE, SELECTED_ACTIVE -> ACTIVE, INACTIVE -> SELECTED_INACTIVE, SELECTED_INACTIVE -> SELECTED_PENDING_MSG_INACTIVE, SELECTED_PENDING_MSG_INACTIVE -> OK
    EXPECT_TRUE(mContactsHandler->deactivate(1));
    EXPECT_TRUE(mContactsHandler->select(1));
    EXPECT_TRUE(mContactsHandler->send(1));
    EXPECT_TRUE(mContactsHandler->send(1));
    // SELECTED_PENDING_MSG_INACTIVE -> PENDING_MSG_INACTIVE, ACTIVE -> SELECTED_ACTIVE, SELECTED_ACTIVE -> OK
    EXPECT_TRUE(mContactsHandler->select(2));
    EXPECT_TRUE(mContactsHandler->receive(2));
    // SELECTED_ACTIVE -> ACTIVE, ACTIVE -> SELECTED_ACTIVE, ACTIVE -> UNREAD_MSG_ACTIVE
    EXPECT_TRUE(mContactsHandler->select(0));
    EXPECT_TRUE(mContactsHandler->receive(2));
    // UNREAD_MSG_ACTIVE -> OK
    EXPECT_TRUE(mContactsHandler->receive(2));
    std::this_thread::sleep_for(std::chrono::milliseconds{TTCONTACTS_HEARTBEAT_TIMEOUT_MS});
    EXPECT_TRUE(StopHandler());
    // Expected data
    EXPECT_GT(mSentMessages.size(), mExpectedMessages.size());
    EXPECT_TRUE(IsFirstEqualTo(mSentMessages, mExpectedMessages.front()));
    EXPECT_TRUE(IsLastEqualTo(mSentMessages, mExpectedMessages.back()));
    EXPECT_TRUE(IsOrderEqualTo({mSentMessages.begin(), mSentMessages.end()}, {mExpectedMessages.begin() + 1, mExpectedMessages.end() - 1}));
    for (size_t i = 0; i < mExpectedEntries.size(); ++i) {
        ASSERT_NE(mContactsHandler->get(i), std::nullopt);
        EXPECT_EQ(mContactsHandler->get(i).value(), mExpectedEntries[i]);
        ASSERT_NE(mContactsHandler->get(mExpectedEntries[i].identity), std::nullopt);
        EXPECT_EQ(mContactsHandler->get(mExpectedEntries[i].identity).value(), i);
    }
    ASSERT_NE(mContactsHandler->current(), std::nullopt);
    EXPECT_EQ(mContactsHandler->current().value(), 0);
    EXPECT_EQ(mContactsHandler->size(), mExpectedEntries.size());
}

// TEST_F(TTContactsHandlerTest, SuccessActiveInactiveMachineState) {
    
// }

// TEST_F(TTContactsHandlerTest, SuccessMixMachineState) {
    
// }

// TEST_F(TTContactsHandlerTest, FailedSelectionMachineState) {
    
// }

// TEST_F(TTContactsHandlerTest, FailedSendAndReceiveMachineState) {
    
// }

// TEST_F(TTContactsHandlerTest, FailedActiveInactiveMachineState) {
    
// }

// TEST_F(TTContactsHandlerTest, FailedMixMachineState) {
    
// }

// Idea for successfull concurrency
// One thread is setting states correctly but other is not
// Example
// T1: Sets active<->inactive
// T1: Sets whatever state id > max