cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsUtils VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)

get_filename_component(TT_ROOT_DIRECTORY "../../" ABSOLUTE)
get_filename_component(TT_DIAGNOSTICS_DIRECTORY "${TT_ROOT_DIRECTORY}/diagnostics" ABSOLUTE)
get_filename_component(TT_UTILS_SRC_DIRECTORY "." ABSOLUTE)
set(TT_DIAGNOSTICS_LIB "tteams-diagnostics")

if (NOT TARGET ${TT_DIAGNOSTICS_LIB})
  add_subdirectory("${TT_DIAGNOSTICS_DIRECTORY}" "${TT_DIAGNOSTICS_LIB}")
endif()

set(TT_UTILS_LIB tteams-utils)
add_library(${TT_UTILS_LIB}
  "${TT_UTILS_SRC_DIRECTORY}/TTUtilsMessageQueue.cpp"
  "${TT_UTILS_SRC_DIRECTORY}/TTUtilsSharedMem.cpp"
)
set_target_properties(${TT_UTILS_LIB} PROPERTIES VERSION ${PROJECT_VERSION})
target_include_directories(${TT_UTILS_LIB} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${TT_UTILS_LIB} PUBLIC "${TT_UTILS_SRC_DIRECTORY}")
target_include_directories (${TT_UTILS_LIB} PRIVATE $<TARGET_PROPERTY:${TT_DIAGNOSTICS_LIB},INTERFACE_INCLUDE_DIRECTORIES>)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  target_link_libraries(${TT_UTILS_LIB} ${TT_DIAGNOSTICS_LIB})
endif()

install(TARGETS ${TT_UTILS_LIB} DESTINATION "lib")
