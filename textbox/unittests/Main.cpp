#include <gtest/gtest.h>
#include "TTDiagnosticsLogger.hpp"

TTDiagnosticsLogger TTDiagnosticsLogger::mInstance("tteams-textbox-ut");

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
