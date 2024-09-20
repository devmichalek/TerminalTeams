cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsEngineUnittests VERSION 1.0)

# Set literals
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(TT_ENGINE_LIB "tteams-engine")
set(TT_ENGINE_UT "tteams-engine-unittests")
get_filename_component(TT_ROOT_DIRECTORY "../../" ABSOLUTE)
get_filename_component(TT_ENGINE_SRC_DIRECTORY "../src" ABSOLUTE)
get_filename_component(TT_DIAGNOSTICS_DIRECTORY "${TT_ROOT_DIRECTORY}/diagnostics/src" ABSOLUTE)
get_filename_component(TT_UTILS_DIRECTORY "${TT_ROOT_DIRECTORY}/utils/src" ABSOLUTE)
get_filename_component(TT_ENGINE_MOCKS_DIRECTORY "../mocks" ABSOLUTE)
get_filename_component(TT_UTILS_MOCKS_DIRECTORY "../../utils/mocks" ABSOLUTE)
get_filename_component(TT_CHAT_MOCKS_DIRECTORY "../../chat/mocks" ABSOLUTE)
get_filename_component(TT_CONTACTS_MOCKS_DIRECTORY "../../contacts/mocks" ABSOLUTE)
get_filename_component(TT_TEXTBOX_MOCKS_DIRECTORY "../../textbox/mocks" ABSOLUTE)
get_filename_component(TT_ENGINE_UNIT_TESTS_DIRECTORY "." ABSOLUTE)

set(TT_ENGINE_UNIT_TESTS
  "${TT_ENGINE_UNIT_TESTS_DIRECTORY}/Main.cpp"
  "${TT_ENGINE_UNIT_TESTS_DIRECTORY}/TTEngineTest.cpp"
  "${TT_ENGINE_UNIT_TESTS_DIRECTORY}/TTEngineSettingsTest.cpp"
  "${TT_ENGINE_UNIT_TESTS_DIRECTORY}/TTBroadcasterChatTest.cpp"
  "${TT_ENGINE_UNIT_TESTS_DIRECTORY}/TTBroadcasterDiscoveryTest.cpp"
  "${TT_ENGINE_UNIT_TESTS_DIRECTORY}/TTNeighborsStubTest.cpp"
  "${TT_ENGINE_UNIT_TESTS_DIRECTORY}/TTNeighborsServiceDiscoveryTest.cpp"
  "${TT_ENGINE_UNIT_TESTS_DIRECTORY}/TTNeighborsServiceChatTest.cpp"
  "${TT_ENGINE_UNIT_TESTS_DIRECTORY}/TTNetworkInterfaceTest.cpp"
)
set(TT_ENGINE_UNIT_TESTS_SCRIPTS "tteams-engine-unittests.sh")
set(TT_ENGINE_DST "unittests")

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
if (NOT TARGET ${TT_ENGINE_LIB})
  add_subdirectory("${TT_ENGINE_SRC_DIRECTORY}" "${TT_ENGINE_LIB}" EXCLUDE_FROM_ALL)
endif()

# Build executable
add_executable(${TT_ENGINE_UT}
  "${TT_ENGINE_UNIT_TESTS}"
)
target_include_directories(${TT_ENGINE_UT} PRIVATE "${TT_ENGINE_SRC_DIRECTORY}")
target_include_directories(${TT_ENGINE_UT} PRIVATE "${TT_ENGINE_MOCKS_DIRECTORY}")
target_include_directories(${TT_ENGINE_UT} PRIVATE "${TT_DIAGNOSTICS_DIRECTORY}")
target_include_directories(${TT_ENGINE_UT} PRIVATE "${TT_UTILS_DIRECTORY}")
target_include_directories(${TT_ENGINE_UT} PRIVATE "${TT_CHAT_MOCKS_DIRECTORY}")
target_include_directories(${TT_ENGINE_UT} PRIVATE "${TT_CONTACTS_MOCKS_DIRECTORY}")
target_include_directories(${TT_ENGINE_UT} PRIVATE "${TT_TEXTBOX_MOCKS_DIRECTORY}")
target_include_directories(${TT_ENGINE_UT} PRIVATE "${TT_ENGINE_UNIT_TESTS_DIRECTORY}")
target_link_libraries(${TT_ENGINE_UT}
  ${TT_ENGINE_LIB}
  GTest::gtest_main
  GTest::gmock_main
)
include(GoogleTest)
gtest_discover_tests(${TT_ENGINE_UT})

# Installation rules
install(TARGETS ${TT_ENGINE_UT} DESTINATION "${TT_ENGINE_DST}")
install(FILES ${TT_ENGINE_UNIT_TESTS_SCRIPTS} DESTINATION "${TT_ENGINE_DST}"
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
)
