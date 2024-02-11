cmake_minimum_required(VERSION 3.22)
project(LocalChat VERSION 1.0)

set(BINARY_NAME lchat)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(ABSL_ENABLE_INSTALL ON)

add_executable(${BINARY_NAME} src/Main.cpp)
install(TARGETS ${BINARY_NAME} DESTINATION install)

# Config file
configure_file(src/Config.hpp.in Config.hpp)
target_include_directories(${BINARY_NAME} PUBLIC "${PROJECT_BINARY_DIR}")

add_subdirectory(./lib/grpc ${CMAKE_CURRENT_BINARY_DIR}/grpc EXCLUDE_FROM_ALL)