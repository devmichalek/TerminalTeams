#include <gtest/gtest.h>
#include "TTDiagnosticsLogger.hpp"

LOG_DECLARE("tteams-chat-unittests");

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
