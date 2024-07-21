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

    bool StartHandler() {
        mContactsHandler.reset(new TTContactsHandler(*mSettingsMock));
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
    std::unique_ptr<TTContactsHandler> mContactsHandler;
    std::vector<TTContactsMessage> mSentMessages;
};

TEST_F(TTContactsHandlerTest, FailedToInitSharedMemory) {
    EXPECT_CALL(*mSharedMemMock, create)
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_THROW(StartHandler(), std::runtime_error);
    EXPECT_FALSE(StopHandler());
}

TEST_F(TTContactsHandlerTest, FailedToEstablishConnection) {
    const auto expectedMessage = CreateMessage(TTContactsStatus::HEARTBEAT);
    EXPECT_CALL(*mSharedMemMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSharedMemMock, send(_, _, _))
        .WillOnce(std::bind(&TTContactsHandlerTest::RetrieveSentMessageFalse, this, _1, _2, _3));
    EXPECT_THROW(StartHandler(), std::runtime_error);
    EXPECT_EQ(mSentMessages.size(), 1);
    EXPECT_TRUE(IsFirstEqualTo(mSentMessages, expectedMessage));
    EXPECT_FALSE(StopHandler());
}

TEST_F(TTContactsHandlerTest, SuccessAtLeastThreeHeartbeatsAndGoodbye) {
    const auto expectedHeartbeat = CreateMessage(TTContactsStatus::HEARTBEAT);
    const auto expectedGoodbye = CreateMessage(TTContactsStatus::GOODBYE);
    EXPECT_CALL(*mSharedMemMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mSharedMemMock, send)
        .Times(AtLeast(3))
        .WillRepeatedly(std::bind(&TTContactsHandlerTest::RetrieveSentMessageTrue, this, _1, _2, _3));
    EXPECT_CALL(*mSharedMemMock, destroy)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(StartHandler());
    std::this_thread::sleep_for(std::chrono::milliseconds(TTCONTACTS_HEARTBEAT_TIMEOUT_MS * 3));
    EXPECT_TRUE(StopHandler());
    EXPECT_GT(mSentMessages.size(), 3);
    EXPECT_TRUE(IsLastEqualTo(mSentMessages, expectedGoodbye));
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end() - 1}, expectedHeartbeat));
}


