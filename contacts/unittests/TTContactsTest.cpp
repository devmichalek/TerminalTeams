#include "TTContacts.hpp"
#include "TTContactsSettingsMock.hpp"
#include "TTContactsConsumerMock.hpp"
#include "TTContactsOutputStreamMock.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>
#include <sstream>
#include <memory>

using ::testing::Test;
using ::testing::Return;

class TTContactsTest : public Test {
 protected:
    TTContactsTest() {
        const char* argv[] = {"", "0", "0", "contacts"};
        mSettingsMock = std::make_shared<TTContactsSettingsMock>(4, argv);
        mConsumerMock = std::make_shared<TTContactsConsumerMock>("", "", "", nullptr);
        mOutputStreamMock = std::make_shared<TTContactsOutputStreamMock>();
    }
    ~TTContactsTest() {

    }
    // Called after constructor, before each test
    virtual void SetUp() override {
        setCallbackQuitFalse();
        EXPECT_CALL(*mSettingsMock, getTerminalWidth).Times(1);
        EXPECT_CALL(*mSettingsMock, getTerminalHeight).Times(1);
        EXPECT_CALL(*mSettingsMock, getConsumer)
            .Times(1)
            .WillOnce(Return(mConsumerMock));
    }
    // Called before destructor, after each test
    virtual void TearDown() override {

    }

    void setCallbackQuitTrue() {
        mCallbackQuitMock = [](){ return true; };
    }

    void setCallbackQuitFalse() {
        mCallbackQuitMock = [](){ return false; };
    }

    void createContacts() {
        mContacts = std::make_unique<TTContacts>(*mSettingsMock, mCallbackQuitMock, *mOutputStreamMock);
    }

    TTContactsCallbackQuit mCallbackQuitMock;
    std::shared_ptr<TTContactsSettingsMock> mSettingsMock;
    std::shared_ptr<TTContactsConsumerMock> mConsumerMock;
    std::shared_ptr<TTContactsOutputStreamMock> mOutputStreamMock;
    std::unique_ptr<TTContacts> mContacts;
};

TEST_F(TTContactsTest, ConsumerInitFailed) {
    EXPECT_CALL(*mConsumerMock, init)
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_THROW(createContacts(), std::runtime_error);
}


