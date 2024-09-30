#include "TTTextBox.hpp"
#include "TTTextBoxSettingsMock.hpp"
#include "TTUtilsNamedPipeMock.hpp"
#include "TTUtilsOutputStreamMock.hpp"
#include "TTUtilsInputStreamMock.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstring>
#include <span>

using ::testing::Test;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::InSequence;
using ::testing::_;
using ::testing::AtLeast;
using namespace std::placeholders;

class TTTextBoxTest : public Test {
public:
    bool RetrieveSentMessageTrue(const void* message) {
        TTTextBoxMessage sentMessage(TTTextBoxStatus::UNDEFINED, 0, nullptr);
        std::memcpy(&sentMessage, message, sizeof(sentMessage));
        mSentMessages.push_back(sentMessage);
        return true;
    }

    bool RetrieveSentMessageFalse(const void* message) {
        TTTextBoxMessage sentMessage(TTTextBoxStatus::UNDEFINED, 0, nullptr);
        std::memcpy(&sentMessage, message, sizeof(sentMessage));
        mSentMessages.push_back(sentMessage);
        return false;
    }
protected:
    TTTextBoxTest() {
        mSettingsMock = std::make_shared<TTTextBoxSettingsMock>();
        mNamedPipeMock = std::make_shared<TTUtilsNamedPipeMock>();
        mOutputStreamMock = std::make_shared<TTUtilsOutputStreamMock>();
        mInputStreamMock = std::make_shared<TTUtilsInputStreamMock>();
    }
    ~TTTextBoxTest() {

    }
    // Called after constructor, before each test
    virtual void SetUp() override {
        EXPECT_CALL(*mSettingsMock, getNamedPipe)
            .Times(1)
            .WillOnce(Return(mNamedPipeMock));
    }
    // Called before destructor, after each test
    virtual void TearDown() override {
        mOutputStreamMock->mOutput.clear();
        mInputStreamMock->clear();
        mSentMessages.clear();
        mExpectedMessages.clear();
    }

    bool IsFirstEqualTo(const std::span<TTTextBoxMessage>& lhs, const TTTextBoxMessage& rhs) const {
        if (lhs.empty()) {
            return false;
        }
        return lhs.front() == rhs;
    }

    bool IsLastEqualTo(const std::span<TTTextBoxMessage>& lhs, const TTTextBoxMessage& rhs) const {
        if (lhs.empty()) {
            return false;
        }
        return lhs.back() == rhs;
    }

    bool IsEachEqualTo(const std::span<TTTextBoxMessage>& lhs, const TTTextBoxMessage& rhs) const {
        if (lhs.empty()) {
            return false;
        }
        if (std::adjacent_find(lhs.begin(), lhs.end(), std::not_equal_to<>()) == lhs.end()) {
            return lhs.front() == rhs;
        }
        return false;
    }

    bool IsAtLeastOneEqualTo(const std::span<TTTextBoxMessage>& lhs, const TTTextBoxMessage& rhs) const {
        if (lhs.empty()) {
            return false;
        }
        return std::find(lhs.begin(), lhs.end(), rhs) != lhs.end();
    }

    bool IsOrderEqualTo(const std::span<TTTextBoxMessage>& lhs, const std::span<TTTextBoxMessage>& rhs) const {
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

    void AddExpectedHeartbeatMessage() {
        mExpectedMessages.emplace_back(TTTextBoxStatus::HEARTBEAT, 0, nullptr);
    }

    void AddExpectedGoodbyeMessage() {
        mExpectedMessages.emplace_back(TTTextBoxStatus::GOODBYE, 0, nullptr);
    }

    void AddExpectedContactsSelectionMessage(size_t id) {
        mExpectedMessages.emplace_back(TTTextBoxStatus::CONTACTS_SELECT, sizeof(id), reinterpret_cast<char*>(&id));
    }

    void AddExpectedMessage(const std::string& msg) {
        mExpectedMessages.emplace_back(TTTextBoxStatus::MESSAGE, msg.size(), msg.c_str());
    }

    void StartApplication() {
        mTextBox->run();
        mApplicationCv.notify_one();
    }

    void RestartApplication(std::chrono::milliseconds timeout) {
        mApplicationTimeout = timeout;
        mTextBox = std::make_unique<TTTextBox>(*mSettingsMock, *mOutputStreamMock, *mInputStreamMock);
        EXPECT_FALSE(mTextBox->isStopped());
        mApplicationThread = std::thread{&TTTextBoxTest::StartApplication, this};
    }

    void VerifyApplicationTimeout() {
        std::unique_lock<std::mutex> lock(mApplicationMutex);
        const bool predicate = mApplicationCv.wait_for(lock, mApplicationTimeout, [this]() {
            return mTextBox->isStopped();
        });
        EXPECT_TRUE(predicate); // Check for application timeout
        EXPECT_TRUE(mTextBox->isStopped());
        mApplicationThread.join();
    }

    std::shared_ptr<TTTextBoxSettingsMock> mSettingsMock;
    std::shared_ptr<TTUtilsNamedPipeMock> mNamedPipeMock;
    std::shared_ptr<TTUtilsOutputStreamMock> mOutputStreamMock;
    std::shared_ptr<TTUtilsInputStreamMock> mInputStreamMock;
    std::unique_ptr<TTTextBox> mTextBox;
    std::chrono::milliseconds mApplicationTimeout;
    std::thread mApplicationThread;
    std::mutex mApplicationMutex;
    std::condition_variable mApplicationCv;
    std::vector<TTTextBoxMessage> mSentMessages;
    std::vector<TTTextBoxMessage> mExpectedMessages;
};

TEST_F(TTTextBoxTest, FailedToCreateNamedPipe) {
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_THROW(RestartApplication(std::chrono::milliseconds{0}), std::runtime_error);
}

TEST_F(TTTextBoxTest, FailedToRunNamedPipeIsNotAlive) {
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_THROW(RestartApplication(std::chrono::milliseconds{0}), std::runtime_error);
}

TEST_F(TTTextBoxTest, SuccessEmptyInput) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    AddExpectedGoodbyeMessage();
    const size_t expectedMinNumOfMessages = 3;
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(expectedMinNumOfMessages))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{600});
    std::this_thread::sleep_for(std::chrono::milliseconds{600 * expectedMinNumOfMessages});
    mTextBox->stop();
    mInputStreamMock->input("");
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    // Verify
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "Type #help to print a help message\n",
        ""
    };
    EXPECT_EQ(actual, expected);
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end() - 1}, mExpectedMessages.front()));
    EXPECT_TRUE(IsLastEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.back()));
}

TEST_F(TTTextBoxTest, FailedEmptyInput) {
    // Expected messages
    const size_t expectedMinNumOfMessages = 1;
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(expectedMinNumOfMessages))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageFalse, this, _1));
    RestartApplication(std::chrono::milliseconds{600});
    std::this_thread::sleep_for(std::chrono::milliseconds{600 * expectedMinNumOfMessages});
    EXPECT_TRUE(mTextBox->isStopped());
    mInputStreamMock->input("");
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    // Verify
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "Type #help to print a help message\n",
        ""
    };
    EXPECT_EQ(actual, expected);
}

TEST_F(TTTextBoxTest, SuccessEmptyCommand) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    AddExpectedGoodbyeMessage();
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{600});
    mInputStreamMock->input("#");
    std::this_thread::sleep_for(std::chrono::milliseconds{600});
    mTextBox->stop();
    mInputStreamMock->input("");
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    // Verify
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "Type #help to print a help message\n",
        "",
        ""
    };
    EXPECT_EQ(actual, expected);
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end() - 1}, mExpectedMessages.front()));
    EXPECT_TRUE(IsLastEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.back()));
}

TEST_F(TTTextBoxTest, SuccessNonExistingCommand) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    AddExpectedGoodbyeMessage();
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{600});
    mInputStreamMock->input("#blahblah");
    std::this_thread::sleep_for(std::chrono::milliseconds{600});
    mTextBox->stop();
    mInputStreamMock->input("");
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    // Verify
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "Type #help to print a help message\n",
        "",
        ""
    };
    EXPECT_EQ(actual, expected);
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end() - 1}, mExpectedMessages.front()));
    EXPECT_TRUE(IsLastEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.back()));
}

TEST_F(TTTextBoxTest, SuccessHelpCommand) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    AddExpectedGoodbyeMessage();
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{600});
    mInputStreamMock->input("#help");
    std::this_thread::sleep_for(std::chrono::milliseconds{600});
    mTextBox->stop();
    mInputStreamMock->input("");
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    // Verify
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "Type #help to print a help message\n",
            "Type #help to print a help message\n" // no comma on purpose
            "Type #quit to quit the application\n" // no comma on purpose
            "Type #select <id> to select contact\n" // no comma on purpose
            "Skip # and send a message to the currently selected contact.\n",
        ""
    };
    EXPECT_EQ(actual, expected);
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end() - 1}, mExpectedMessages.front()));
    EXPECT_TRUE(IsLastEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.back()));
}

TEST_F(TTTextBoxTest, FailureHelpCommandTooManyArguments) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    AddExpectedGoodbyeMessage();
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{600});
    mInputStreamMock->input("#help blahblah");
    std::this_thread::sleep_for(std::chrono::milliseconds{600});
    mTextBox->stop();
    mInputStreamMock->input("");
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    // Verify
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "Type #help to print a help message\n",
        "",
        ""
    };
    EXPECT_EQ(actual, expected);
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end() - 1}, mExpectedMessages.front()));
    EXPECT_TRUE(IsLastEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.back()));
}

TEST_F(TTTextBoxTest, SuccessQuitCommand) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    AddExpectedGoodbyeMessage();
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{600});
    std::this_thread::sleep_for(std::chrono::milliseconds{600});
    mInputStreamMock->input("#quit");
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    // Verify
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "Type #help to print a help message\n",
        ""
    };
    EXPECT_EQ(actual, expected);
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end() - 1}, mExpectedMessages.front()));
    EXPECT_TRUE(IsLastEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.back()));
}

TEST_F(TTTextBoxTest, FailureQuitCommandTooManyArguments) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    AddExpectedGoodbyeMessage();
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{600});
    mInputStreamMock->input("#quit blahblah");
    std::this_thread::sleep_for(std::chrono::milliseconds{600});
    mTextBox->stop();
    mInputStreamMock->input("");
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    // Verify
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "Type #help to print a help message\n",
        "",
        ""
    };
    EXPECT_EQ(actual, expected);
    EXPECT_GE(mSentMessages.size(), 2);
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end() - 1}, mExpectedMessages.front()));
    EXPECT_TRUE(IsLastEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.back()));
}

TEST_F(TTTextBoxTest, SuccessSelectCommand) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    AddExpectedContactsSelectionMessage(1);
    AddExpectedGoodbyeMessage();
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{600});
    std::this_thread::sleep_for(std::chrono::milliseconds{600});
    mInputStreamMock->input("#select 1");
    std::this_thread::sleep_for(std::chrono::milliseconds{1000});
    mTextBox->stop();
    mInputStreamMock->input("");
    // Verify
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "Type #help to print a help message\n",
        "",
        ""
    };
    EXPECT_EQ(actual, expected);
    EXPECT_TRUE(IsFirstEqualTo({mSentMessages.begin(), mSentMessages.end() - 1}, mExpectedMessages.front()));
    EXPECT_TRUE(IsLastEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.back()));
    EXPECT_TRUE(IsAtLeastOneEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages[1]));
}

TEST_F(TTTextBoxTest, FailureSelectCommandTooManyArguments) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    AddExpectedGoodbyeMessage();
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{600});
    mInputStreamMock->input("#select 1 blahblah");
    std::this_thread::sleep_for(std::chrono::milliseconds{600});
    mTextBox->stop();
    mInputStreamMock->input("");
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    // Verify
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "Type #help to print a help message\n",
        "",
        ""
    };
    EXPECT_EQ(actual, expected);
    EXPECT_GE(mSentMessages.size(), 2);
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end() - 1}, mExpectedMessages.front()));
    EXPECT_TRUE(IsLastEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.back()));
}

TEST_F(TTTextBoxTest, FailureSelectCommandNoDigits) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    AddExpectedGoodbyeMessage();
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{600});
    mInputStreamMock->input("#select blahblah");
    std::this_thread::sleep_for(std::chrono::milliseconds{600});
    mTextBox->stop();
    mInputStreamMock->input("");
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    // Verify
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "Type #help to print a help message\n",
        "",
        ""
    };
    EXPECT_EQ(actual, expected);
    EXPECT_GE(mSentMessages.size(), 2);
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end() - 1}, mExpectedMessages.front()));
    EXPECT_TRUE(IsLastEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.back()));
}

TEST_F(TTTextBoxTest, FailureSelectCommandTooManyDigits) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    AddExpectedGoodbyeMessage();
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{600});
    mInputStreamMock->input("#select 11111");
    std::this_thread::sleep_for(std::chrono::milliseconds{600});
    mTextBox->stop();
    mInputStreamMock->input("");
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    // Verify
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "Type #help to print a help message\n",
        "",
        ""
    };
    EXPECT_EQ(actual, expected);
    EXPECT_GE(mSentMessages.size(), 2);
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end() - 1}, mExpectedMessages.front()));
    EXPECT_TRUE(IsLastEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.back()));
}

TEST_F(TTTextBoxTest, SuccessSmallMessage) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    const std::string oneCharMsg = "a";
    const std::string customMsg = "Some custom message  with  spaces and   tab";
    const std::string upToLimitMsg(TTTextBoxMessage::DATA_MAX_LENGTH, 'x');
    AddExpectedMessage(oneCharMsg);
    AddExpectedMessage(customMsg);
    AddExpectedMessage(upToLimitMsg);
    AddExpectedGoodbyeMessage();
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{600});
    std::this_thread::sleep_for(std::chrono::milliseconds{600});
    mInputStreamMock->input(oneCharMsg);
    mInputStreamMock->input(customMsg);
    mInputStreamMock->input(upToLimitMsg);
    std::this_thread::sleep_for(std::chrono::milliseconds{1000});
    mTextBox->stop();
    mInputStreamMock->input("");
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    // Verify
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "Type #help to print a help message\n",
        "",
        "",
        "",
        ""
    };
    EXPECT_EQ(actual, expected);
    EXPECT_TRUE(IsFirstEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.front()));
    EXPECT_TRUE(IsLastEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.back()));
    EXPECT_TRUE(IsOrderEqualTo({mSentMessages.begin(), mSentMessages.end()}, {mExpectedMessages.begin() + 1, mExpectedMessages.end() - 1}));
}

TEST_F(TTTextBoxTest, SuccessBigMessage) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    const std::string bigMessage1(TTTextBoxMessage::DATA_MAX_LENGTH * 2, 'y');
    const std::string bigMessageChunk1(TTTextBoxMessage::DATA_MAX_LENGTH, 'y');
    const std::string bigMessage2(TTTextBoxMessage::DATA_MAX_LENGTH * 10, 'z');
    const std::string bigMessageChunk2(TTTextBoxMessage::DATA_MAX_LENGTH, 'z');
    const std::string bigMessage3(TTTextBoxMessage::DATA_MAX_LENGTH * 1.5, 'm');
    const std::string bigMessageChunk3a(TTTextBoxMessage::DATA_MAX_LENGTH, 'm');
    const std::string bigMessageChunk3b(TTTextBoxMessage::DATA_MAX_LENGTH / 2, 'm');
    AddExpectedMessage(bigMessageChunk1);
    AddExpectedMessage(bigMessageChunk1);
    AddExpectedMessage(bigMessageChunk2);
    AddExpectedMessage(bigMessageChunk2);
    AddExpectedMessage(bigMessageChunk2);
    AddExpectedMessage(bigMessageChunk2);
    AddExpectedMessage(bigMessageChunk2);
    AddExpectedMessage(bigMessageChunk3a);
    AddExpectedMessage(bigMessageChunk3b);
    AddExpectedGoodbyeMessage();
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{600});
    std::this_thread::sleep_for(std::chrono::milliseconds{1000});
    mInputStreamMock->input(bigMessage1);
    mInputStreamMock->input(bigMessage2);
    mInputStreamMock->input(bigMessage3);
    std::this_thread::sleep_for(std::chrono::milliseconds{600});
    mTextBox->stop();
    mInputStreamMock->input("");
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    // Verify
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "Type #help to print a help message\n",
        "",
        "",
        "",
        ""
    };
    EXPECT_EQ(actual, expected);
    EXPECT_TRUE(IsFirstEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.front()));
    EXPECT_TRUE(IsLastEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.back()));
    EXPECT_TRUE(IsOrderEqualTo({mSentMessages.begin(), mSentMessages.end()}, {mExpectedMessages.begin() + 1, mExpectedMessages.end() - 1}));
}

TEST_F(TTTextBoxTest, SuccessMixOfMessagesAndCommands) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    AddExpectedContactsSelectionMessage(1);
    const std::string customMsg = "Hello world";
    AddExpectedMessage(customMsg);
    AddExpectedGoodbyeMessage();
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, create)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{600});
    std::this_thread::sleep_for(std::chrono::milliseconds{600});
    mInputStreamMock->input("#help");
    mInputStreamMock->input("#select 1");
    mInputStreamMock->input(customMsg);
    std::this_thread::sleep_for(std::chrono::milliseconds{1000});
    mInputStreamMock->input("#quit");
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    // Verify
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "Type #help to print a help message\n",
            "Type #help to print a help message\n" // no comma on purpose
            "Type #quit to quit the application\n" // no comma on purpose
            "Type #select <id> to select contact\n" // no comma on purpose
            "Skip # and send a message to the currently selected contact.\n",
        "",
        "",
        ""
    };
    EXPECT_EQ(actual, expected);
    EXPECT_TRUE(IsFirstEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.front()));
    EXPECT_TRUE(IsLastEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.back()));
    EXPECT_TRUE(IsOrderEqualTo({mSentMessages.begin(), mSentMessages.end()}, {mExpectedMessages.begin() + 1, mExpectedMessages.end() - 1}));
}
