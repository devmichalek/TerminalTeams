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

    void AddExpectedContactsSwitchMessage(size_t id) {
        mExpectedMessages.emplace_back(TTTextBoxStatus::CONTACTS_SWITCH, sizeof(id), reinterpret_cast<char*>(&id));
    }

    void StartApplication() {
        mTextBox->run();
        mApplicationCv.notify_one();
    }

    void RestartApplication(std::chrono::milliseconds timeout) {
        mApplicationTimeout = timeout;
        mTextBox = std::make_unique<TTTextBox>(*mSettingsMock, *mOutputStreamMock, *mInputStreamMock);
        EXPECT_FALSE(mTextBox->stopped());
        mApplicationThread = std::thread{&TTTextBoxTest::StartApplication, this};
    }

    void VerifyApplicationTimeout() {
        std::unique_lock<std::mutex> lock(mApplicationMutex);
        const bool predicate = mApplicationCv.wait_for(lock, mApplicationTimeout, [this]() {
            return mTextBox->stopped();
        });
        EXPECT_TRUE(predicate); // Check for application timeout
        EXPECT_TRUE(mTextBox->stopped());
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

TEST_F(TTTextBoxTest, FailedToOpenNamedPipe) {
    EXPECT_CALL(*mNamedPipeMock, open)
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_THROW(RestartApplication(std::chrono::milliseconds{0}), std::runtime_error);
}

TEST_F(TTTextBoxTest, FailedToRunNamedPipeIsNotAlive) {
    EXPECT_CALL(*mNamedPipeMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(2)
        .WillRepeatedly(Return(false));
    mTextBox = std::make_unique<TTTextBox>(*mSettingsMock, *mOutputStreamMock, *mInputStreamMock);
    mTextBox->run();
    EXPECT_TRUE(mTextBox->stopped());
}

TEST_F(TTTextBoxTest, SuccessEmptyInput) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    const size_t expectedMinNumOfMessages = 3;
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(2)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(expectedMinNumOfMessages))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{500});
    std::this_thread::sleep_for(std::chrono::milliseconds{600 * expectedMinNumOfMessages});
    mTextBox->stop();
    mInputStreamMock->input("");
    // Verify
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "Type #help to print a help message\n",
        ""
    };
    EXPECT_EQ(actual, expected);
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.front()));
}

TEST_F(TTTextBoxTest, SuccessEmptyCommand) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(2)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{500});
    mInputStreamMock->input("#");
    std::this_thread::sleep_for(std::chrono::milliseconds{600});
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
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.front()));
}

TEST_F(TTTextBoxTest, SuccessNonExistingCommand) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(2)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{500});
    mInputStreamMock->input("#blahblah");
    std::this_thread::sleep_for(std::chrono::milliseconds{600});
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
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.front()));
}

TEST_F(TTTextBoxTest, SuccessHelpCommand) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(2)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{500});
    mInputStreamMock->input("#help");
    std::this_thread::sleep_for(std::chrono::milliseconds{600});
    mTextBox->stop();
    mInputStreamMock->input("");
    // Verify
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "Type #help to print a help message\n",
            "Type #help to print a help message\n" // no comma on purpose
            "Type #quit to quit the application\n" // no comma on purpose
            "Type #switch <id> to switch contacts\n" // no comma on purpose
            "Skip # to send a message to the currently selected contact.\n",
        ""
    };
    EXPECT_EQ(actual, expected);
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.front()));
}

TEST_F(TTTextBoxTest, FailureHelpCommandTooManyArguments) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(2)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{500});
    mInputStreamMock->input("#help blahblah");
    std::this_thread::sleep_for(std::chrono::milliseconds{600});
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
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.front()));
}

TEST_F(TTTextBoxTest, SuccessQuitCommand) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(2)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{500});
    std::this_thread::sleep_for(std::chrono::milliseconds{600});
    mInputStreamMock->input("#quit");
    // Verify
    VerifyApplicationTimeout();
    const auto& actual = mOutputStreamMock->mOutput;
    const auto& expected = std::vector<std::string>{
        "Type #help to print a help message\n",
        ""
    };
    EXPECT_EQ(actual, expected);
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.front()));
}

TEST_F(TTTextBoxTest, FailureQuitCommandTooManyArguments) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(2)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{500});
    mInputStreamMock->input("#quit blahblah");
    std::this_thread::sleep_for(std::chrono::milliseconds{600});
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
    EXPECT_TRUE(IsEachEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.front()));
}

TEST_F(TTTextBoxTest, SuccessSwitchCommand) {
    // Expected messages
    AddExpectedHeartbeatMessage();
    AddExpectedContactsSwitchMessage(1);
    // Expected flow
    EXPECT_CALL(*mNamedPipeMock, open)
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mNamedPipeMock, alive)
        .Times(2)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mNamedPipeMock, send)
        .Times(AtLeast(1))
        .WillRepeatedly(std::bind(&TTTextBoxTest::RetrieveSentMessageTrue, this, _1));
    RestartApplication(std::chrono::milliseconds{500});
    std::this_thread::sleep_for(std::chrono::milliseconds{600});
    mInputStreamMock->input("#switch 1");
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
    EXPECT_TRUE(IsFirstEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.front()));
    EXPECT_TRUE(IsLastEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.front()));
    EXPECT_TRUE(IsAtLeastOneEqualTo({mSentMessages.begin(), mSentMessages.end()}, mExpectedMessages.back()));
}

// TEST_F(TTTextBoxTest, FailureSwitchCommandTooManyArguments) {

// }

// TEST_F(TTTextBoxTest, FailureSwitchCommandNoDigits) {

// }

// TEST_F(TTTextBoxTest, FailureSwitchCommandTooManyDigits) {

// }

// TEST_F(TTTextBoxTest, SuccessOneMessage) {

// }

// TEST_F(TTTextBoxTest, SuccessFragmenetedMessage) {

// }

// TEST_F(TTTextBoxTest, SuccessMixOfMessagsAndCommands) {

// }
