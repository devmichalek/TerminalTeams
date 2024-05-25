#include "TTContacts.hpp"
#include "TTContactsSettingsMock.hpp"
#include "TTUtilsSharedMemMock.hpp"
#include "TTUtilsOutputStreamMock.hpp"
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
        mSharedMemMock = std::make_shared<TTUtilsSharedMemMock>();
        mOutputStreamMock = std::make_shared<TTUtilsOutputStreamMock>();
    }
    ~TTContactsTest() {

    }
    // Called after constructor, before each test
    virtual void SetUp() override {
        EXPECT_CALL(*mSettingsMock, getTerminalWidth).Times(1);
        EXPECT_CALL(*mSettingsMock, getTerminalHeight).Times(1);
        EXPECT_CALL(*mSettingsMock, getSharedMemory)
            .Times(1)
            .WillOnce(Return(mSharedMemMock));
    }
    // Called before destructor, after each test
    virtual void TearDown() override {

    }

    void createContacts() {
        mContacts = std::make_unique<TTContacts>(*mSettingsMock, *mOutputStreamMock);
    }

    std::shared_ptr<TTContactsSettingsMock> mSettingsMock;
    std::shared_ptr<TTUtilsSharedMemMock> mSharedMemMock;
    std::shared_ptr<TTUtilsOutputStreamMock> mOutputStreamMock;
    std::unique_ptr<TTContacts> mContacts;
};

TEST_F(TTContactsTest, SharedMemoryInitFailed) {
    EXPECT_CALL(*mSharedMemMock, open)
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_THROW(createContacts(), std::runtime_error);
}


