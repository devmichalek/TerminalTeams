cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsUtilsUnitTests VERSION 1.0)

# Set literals
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(TT_UTILS_LIB tteams-utils)
set(TT_UTILS_UT_LIB tteams-utils-unittests)
set(TT_UTILS_UT_EXE tteams-utils-unittests-exe)
get_filename_component(TT_UTILS_TESTED_DIRECTORY "../src" ABSOLUTE)
get_filename_component(TT_UTILS_UNIT_TESTS_DIRECTORY "." ABSOLUTE)
set(TT_UTILS_UNIT_TESTS "${TT_UTILS_UNIT_TESTS_DIRECTORY}/Main.cpp")
set(TT_UTILS_UNIT_TESTS_SCRIPTS "tteams-utils-unittests.sh")
set(TT_UTILS_DST "unittests")

# Resolve dependencies
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
enable_testing()
if (NOT TARGET ${TT_UTILS_LIB})
  add_subdirectory(${TT_UTILS_TESTED_DIRECTORY} ${TT_UTILS_LIB} EXCLUDE_FROM_ALL)
endif()

# Build executable
add_executable(${TT_UTILS_UT}
  "${TT_UTILS_UNIT_TESTS}"
)
set_target_properties(${TT_UTILS_UT_EXE} PROPERTIES OUTPUT_NAME ${TT_UTILS_UT_LIB})
target_include_directories(${TT_UTILS_UT} PRIVATE "${TT_UTILS_TESTED_DIRECTORY}")
target_include_directories(${TT_UTILS_UT} PUBLIC "${TT_UTILS_UNIT_TESTS_DIRECTORY}")
target_link_libraries(
  ${TT_UTILS_UT}
  ${TT_UTILS_LIB}
  GTest::gtest_main
  GTest::gmock_main
)
include(GoogleTest)
gtest_discover_tests(${TT_UTILS_UT})

# Installation rules
install(TARGETS ${TT_UTILS_UT} DESTINATION "${TT_UTILS_DST}")
install(FILES ${TT_UTILS_UNIT_TESTS_SCRIPTS} DESTINATION "${TT_UTILS_DST}"
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
)
