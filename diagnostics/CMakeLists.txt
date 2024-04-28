cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsDiagnostics VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(Threads REQUIRED)

include(FetchContent)

FetchContent_Declare(
  spdlog
  URL https://github.com/gabime/spdlog/archive/refs/tags/v1.14.0.zip
)
#FetchContent_MakeAvailable(spdlog)
# Hack to make sure fetched content install targets are excluded
FetchContent_GetProperties(spdlog)
if(NOT spdlog_POPULATED)
  FetchContent_Populate(spdlog)
  add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_Declare(
  minitrace
  URL https://github.com/hrydgard/minitrace/archive/refs/heads/master.zip
)
#FetchContent_MakeAvailable(minitrace)
# Hack to make sure fetched content install targets are excluded
FetchContent_GetProperties(minitrace)
if(NOT minitrace_POPULATED)
  FetchContent_Populate(minitrace)
  add_subdirectory(${minitrace_SOURCE_DIR} ${minitrace_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

get_filename_component(TT_DIAGNOSTICS_SRC_DIRECTORY "./src" ABSOLUTE)

set(TT_DIAGNOSTICS tteams-diagnostics)
add_library(${TT_DIAGNOSTICS} INTERFACE)

set_target_properties(${TT_DIAGNOSTICS} PROPERTIES OUTPUT_NAME ${TT_DIAGNOSTICS})
target_include_directories(${TT_DIAGNOSTICS} INTERFACE "${TT_DIAGNOSTICS_SRC_DIRECTORY}")
target_include_directories(${TT_DIAGNOSTICS} INTERFACE "${minitrace_SOURCE_DIR}")
target_include_directories(${TT_DIAGNOSTICS} INTERFACE "${spdlog_SOURCE_DIR}")

target_compile_definitions(${TT_DIAGNOSTICS} INTERFACE "$<$<CONFIG:DEBUG>:TT_DIAGNOSTICS_TRACER_DEBUG>")
target_compile_definitions(${TT_DIAGNOSTICS} INTERFACE "$<$<CONFIG:DEBUG>:TT_DIAGNOSTICS_LOGGER_DEBUG>")

target_link_libraries(
  ${TT_DIAGNOSTICS} INTERFACE
  minitrace
  spdlog
)
