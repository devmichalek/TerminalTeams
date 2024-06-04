cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsEngineBundle VERSION 1.0)

set(CMAKE_CXX_FLAGS_RELEASE_INIT "${CMAKE_CXX_FLAGS_RELEASE_INIT} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "${CMAKE_CXX_FLAGS_DEBUG_INIT} -Wall")

add_subdirectory("./src")
