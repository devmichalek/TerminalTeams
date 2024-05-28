cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsChatBundle VERSION 1.0)

set(CMAKE_CXX_FLAGS_RELEASE_INIT "${CMAKE_CXX_FLAGS_RELEASE_INIT} -Wall -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "${CMAKE_CXX_FLAGS_DEBUG_INIT} -Wall -fno-strict-aliasing")

add_subdirectory("./src")
add_subdirectory("./autotests")
add_subdirectory("./unittests")
