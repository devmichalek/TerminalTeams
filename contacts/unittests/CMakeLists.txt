cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsContactsUnitTests VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

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

set(TT_CONTACTS_UT tteams-contacts-ut)
get_filename_component(TT_CONTACTS_TESTED_DIRECTORY "../src" ABSOLUTE)
get_filename_component(TT_CONTACTS_UNIT_TESTS_DIRECTORY "." ABSOLUTE)

set(TT_CONTACTS_TESTED_FILES
  "${TT_CONTACTS_TESTED_DIRECTORY}/TTContacts.cpp"
  "${TT_CONTACTS_TESTED_DIRECTORY}/TTContactsSettings.cpp"
  "${TT_CONTACTS_TESTED_DIRECTORY}/TTContactsConsumer.cpp"
  "${TT_CONTACTS_TESTED_DIRECTORY}/TTContactsHandler.cpp"
)

set(TT_CONTACTS_UNIT_TESTS
  "${TT_CONTACTS_UNIT_TESTS_DIRECTORY}/TTContactsSettingsTest.cpp"
  "${TT_CONTACTS_UNIT_TESTS_DIRECTORY}/TTContactsTest.cpp"
)

set(TT_CONTACTS_UNIT_TESTS_SCRIPTS
  "unittests.sh"
)

add_executable(
  ${TT_CONTACTS_UT}
  "${TT_CONTACTS_TESTED_FILES}"
  "${TT_CONTACTS_UNIT_TESTS}"
)

target_include_directories(${TT_CONTACTS_UT} PUBLIC "${TT_CONTACTS_TESTED_DIRECTORY}")
target_include_directories(${TT_CONTACTS_UT} PUBLIC "${TT_CONTACTS_UNIT_TESTS_DIRECTORY}")

target_link_libraries(
  ${TT_CONTACTS_UT}
  GTest::gtest_main
  GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(${TT_CONTACTS_UT})

install(TARGETS ${TT_CONTACTS_UT} DESTINATION .)
install(FILES ${TT_CONTACTS_UNIT_TESTS_SCRIPTS} DESTINATION .
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ)
