#include "TTContacts.hpp"
#include "TTContactsSettingsMock.hpp"
#include "TTContactsConsumerMock.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>

using ::testing::Test;

class TTContactsTest : public Test {
 protected:
    TTContactsTest() {

    }
    ~TTContactsTest() {

    }
    // Called after constructor, before each test
    virtual void SetUp() override {

    }
    // Called before destructor, after each test
    virtual void TearDown() override {

    }

    void insertMessage();
}

class TTContactsBuffer {

};

TEST_F(TTContactsTest, HappyPath) {

}
