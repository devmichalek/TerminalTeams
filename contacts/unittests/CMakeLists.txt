cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsContactsUnitTests VERSION 1.0)

# Set literals
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(TT_CONTACTS_LIB tteams-contacts)
set(TT_CONTACTS_HANDLER_LIB tteams-contacts-handler)
set(TT_CONTACTS_UT tteams-contacts-unittests)
get_filename_component(TT_ROOT_DIRECTORY "../../" ABSOLUTE)
get_filename_component(TT_CONTACTS_TESTED_DIRECTORY "../src" ABSOLUTE)
get_filename_component(TT_CONTACTS_UNIT_TESTS_DIRECTORY "." ABSOLUTE)
set(TT_CONTACTS_UNIT_TESTS
  "${TT_CONTACTS_UNIT_TESTS_DIRECTORY}/Main.cpp"
  "${TT_CONTACTS_UNIT_TESTS_DIRECTORY}/TTContactsSettingsTest.cpp"
  "${TT_CONTACTS_UNIT_TESTS_DIRECTORY}/TTContactsTest.cpp"
)
set(TT_CONTACTS_UNIT_TESTS_SCRIPTS "tteams-contacts-unittests.sh")
set(TT_CONTACTS_DST "unittests")

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
if (NOT TARGET ${TT_CONTACTS_LIB})
  add_subdirectory(${TT_CONTACTS_TESTED_DIRECTORY} ${TT_CONTACTS_LIB} EXCLUDE_FROM_ALL)
endif()

# Build executable
add_executable(${TT_CONTACTS_UT}
  "${TT_CONTACTS_UNIT_TESTS}"
)
target_include_directories(${TT_CONTACTS_UT} PRIVATE "${TT_CONTACTS_TESTED_DIRECTORY}")
target_include_directories(${TT_CONTACTS_UT} PUBLIC "${TT_CONTACTS_UNIT_TESTS_DIRECTORY}")
target_link_libraries(${TT_CONTACTS_UT}
  ${TT_CONTACTS_LIB}
  ${TT_CONTACTS_HANDLER_LIB}
  GTest::gtest_main
  GTest::gmock_main
)
include(GoogleTest)
gtest_discover_tests(${TT_CONTACTS_UT})

# Installation rules
install(TARGETS ${TT_CONTACTS_UT} DESTINATION "${TT_CONTACTS_DST}")
install(FILES ${TT_CONTACTS_UNIT_TESTS_SCRIPTS} DESTINATION "${TT_CONTACTS_DST}"
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
)
