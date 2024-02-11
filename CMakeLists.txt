cmake_minimum_required(VERSION 3.22)
project(LocalChat VERSION 1.0)
add_executable(${CMAKE_PROJECT_NAME} src/Main.cpp)
install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION install)

# Required C++ version
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Config file
configure_file(src/Config.hpp.in Config.hpp)
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")

