#include "TTContactsHandler.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::Test;

class TTContactsHandlerTest : public Test {
 protected:
    TTContactsHandlerTest() {

    }
    ~TTContactsHandlerTest() {

    }

    // void create() {
    //     mContacts = std::make_unique<TTContacts>(*mSettingsMock, *mOutputStreamMock);
    // }

    //std::shared_ptr<TTContactsSettingsMock> mSettingsMock;
};

// TEST_F(TTContactsHandlerTest, SharedMemoryInitFailed) {
//     EXPECT_CALL(*mSharedMemMock, open)
//         .Times(1)
//         .WillOnce(Return(false));
//     EXPECT_THROW(create(), std::runtime_error);
// }
