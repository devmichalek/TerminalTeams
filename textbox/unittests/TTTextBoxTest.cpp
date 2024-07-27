#include "TTTextBox.hpp"
#include "TTTextBoxSettingsMock.hpp"
#include "TTUtilsNamedPipeMock.hpp"
#include "TTUtilsOutputStreamMock.hpp"
#include "TTUtilsInputStreamMock.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::Test;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::InSequence;
using ::testing::_;

class TTTextBoxTest : public Test {
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
        mInputStreamMock->mInput.clear();
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
};

TEST_F(TTTextBoxTest, FailedToOpenNamedPipe) {
    EXPECT_CALL(*mNamedPipeMock, open)
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_THROW(RestartApplication(std::chrono::milliseconds{0}), std::runtime_error);
}


