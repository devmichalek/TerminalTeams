#include "TTContacts.hpp"
#include "TTContactsSettingsMock.hpp"
#include "TTContactsConsumerMock.hpp"
#include "TTContactsOutputStream.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>
#include <sstream>

using ::testing::Test;

class TTContactsTest : public Test {
 protected:
    TTContactsTest() {

    }
    ~TTContactsTest() {

    }
    // Called after constructor, before each test
    virtual void SetUp() override {
        EXPECT_CALL(mSettingsMock, getTerminalWidth()).Times(1);
        EXPECT_CALL(mSettingsMock, getTerminalHeight()).Times(1);
        EXPECT_CALL(mSettingsMock, getConsumer()).Times(1);
    }
    // Called before destructor, after each test
    virtual void TearDown() override {

    }

    std::shared_ptr<TTContactsSettingsMock> mSettingsMock;
    std::shared_ptr<TTContactsConsumerMock> mConsumerMock;
    std::shared_ptr<TTContactsOutputStreamMock> mOutputStreamMock;
}

TEST_F(TTContactsTest, HappyPath) {

}
