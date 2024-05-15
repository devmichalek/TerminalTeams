cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsDiagnostics VERSION 1.0)

# Set literals
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
set(TT_DIAGNOSTICS_LIB tteams-diagnostics)
get_filename_component(TT_DIAGNOSTICS_SRC_DIRECTORY "." ABSOLUTE)

# Resolve dependencies
find_package(Threads REQUIRED)
include(FetchContent)
FetchContent_Declare(
  spdlog
  URL https://github.com/gabime/spdlog/archive/refs/tags/v1.14.0.zip
)
FetchContent_GetProperties(spdlog)
if(NOT spdlog_POPULATED)
  FetchContent_Populate(spdlog)
  add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
FetchContent_Declare(
  minitrace
  URL https://github.com/hrydgard/minitrace/archive/refs/heads/master.zip
)
FetchContent_GetProperties(minitrace)
if(NOT minitrace_POPULATED)
  FetchContent_Populate(minitrace)
  add_subdirectory(${minitrace_SOURCE_DIR} ${minitrace_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# Build library
add_library(${TT_DIAGNOSTICS_LIB} INTERFACE)
set_target_properties(${TT_DIAGNOSTICS_LIB} PROPERTIES OUTPUT_NAME ${TT_DIAGNOSTICS_LIB})
target_include_directories(${TT_DIAGNOSTICS_LIB} INTERFACE "${TT_DIAGNOSTICS_SRC_DIRECTORY}")
target_include_directories(${TT_DIAGNOSTICS_LIB} INTERFACE "${minitrace_SOURCE_DIR}")
target_include_directories(${TT_DIAGNOSTICS_LIB} INTERFACE "${spdlog_SOURCE_DIR}")
target_compile_definitions(${TT_DIAGNOSTICS_LIB} INTERFACE "$<$<CONFIG:DEBUG>:TT_DIAGNOSTICS_TRACER_DEBUG>")
target_compile_definitions(${TT_DIAGNOSTICS_LIB} INTERFACE "$<$<CONFIG:DEBUG>:TT_DIAGNOSTICS_LOGGER_DEBUG>")
target_link_libraries(${TT_DIAGNOSTICS_LIB} INTERFACE
  minitrace
  spdlog
)
