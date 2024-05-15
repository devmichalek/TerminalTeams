cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsTextboxUnitTests VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
#FetchContent_MakeAvailable(googletest)
# Hack to make sure fetched content install targets are excluded
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

enable_testing()

set(TT_TEXTBOX_LIB tteams-textbox)
set(TT_TEXTBOX_HANDLER_LIB tteams-textbox-handler)
set(TT_TEXTBOX_UT tteams-textbox-unittests)
get_filename_component(TT_TEXTBOX_TESTED_DIRECTORY "../src" ABSOLUTE)
get_filename_component(TT_TEXTBOX_UNIT_TESTS_DIRECTORY "." ABSOLUTE)

if (NOT TARGET ${TT_TEXTBOX_LIB})
  add_subdirectory(${TT_TEXTBOX_TESTED_DIRECTORY} ${TT_TEXTBOX_LIB} EXCLUDE_FROM_ALL)
endif()

set(TT_TEXTBOX_UNIT_TESTS
  "${TT_TEXTBOX_UNIT_TESTS_DIRECTORY}/Main.cpp"
)

set(TT_TEXTBOX_UNIT_TESTS_SCRIPTS
  "tteams-textbox-unittests.sh"
)

add_executable(
  ${TT_TEXTBOX_UT}
  "${TT_TEXTBOX_UNIT_TESTS}"
)

target_include_directories(${TT_TEXTBOX_UT} PRIVATE "${TT_TEXTBOX_TESTED_DIRECTORY}")
target_include_directories(${TT_TEXTBOX_UT} PUBLIC "${TT_TEXTBOX_UNIT_TESTS_DIRECTORY}")

target_link_libraries(
  ${TT_TEXTBOX_UT}
  ${TT_TEXTBOX_LIB}
  ${TT_TEXTBOX_HANDLER_LIB}
  GTest::gtest_main
  GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(${TT_TEXTBOX_UT})

set(TT_TEXTBOX_DST "unittests")
install(TARGETS ${TT_TEXTBOX_UT} DESTINATION "${TT_TEXTBOX_DST}")
install(FILES ${TT_TEXTBOX_UNIT_TESTS_SCRIPTS} DESTINATION "${TT_TEXTBOX_DST}"
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
)
