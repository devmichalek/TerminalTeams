cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsChatsUnitTests VERSION 1.0)

# Set literals
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(TT_CHAT_LIB "tteams-chat")
set(TT_CHAT_HANDLER_LIB "tteams-chat-handler")
set(TT_CHAT_UT "tteams-chat-unittests")
get_filename_component(TT_CHAT_TESTED_DIRECTORY "../src" ABSOLUTE)
get_filename_component(TT_CHAT_UNIT_TESTS_DIRECTORY "." ABSOLUTE)
set(TT_CHAT_UNIT_TESTS
  "${TT_CHAT_UNIT_TESTS_DIRECTORY}/Main.cpp"
)
set(TT_CHAT_UNIT_TESTS_SCRIPTS "tteams-chat-unittests.sh")
set(TT_CHAT_DST "unittests")

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
if (NOT TARGET ${TT_CHAT_LIB})
  add_subdirectory(${TT_CHAT_TESTED_DIRECTORY} ${TT_CHAT_LIB} EXCLUDE_FROM_ALL)
endif()

# Build executable
add_executable(
  ${TT_CHAT_UT}
  "${TT_CHAT_UNIT_TESTS}"
)
target_include_directories(${TT_CHAT_UT} PRIVATE "${TT_CHAT_TESTED_DIRECTORY}")
target_include_directories(${TT_CHAT_UT} PUBLIC "${TT_CHAT_UNIT_TESTS_DIRECTORY}")
target_link_libraries(
  ${TT_CHAT_UT}
  ${TT_CHAT_LIB}
  ${TT_CHAT_HANDLER_LIB}
  GTest::gtest_main
  GTest::gmock_main
)
include(GoogleTest)
gtest_discover_tests(${TT_CHAT_UT})

# Installation rules
install(TARGETS ${TT_CHAT_UT} DESTINATION "${TT_CHAT_DST}")
install(FILES ${TT_CHAT_UNIT_TESTS_SCRIPTS} DESTINATION "${TT_CHAT_DST}"
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
)
