cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsTextBox VERSION 1.0)

# Set literals
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
get_filename_component(TT_ROOT_DIRECTORY "../../" ABSOLUTE)
get_filename_component(TT_DIAGNOSTICS_DIRECTORY "${TT_ROOT_DIRECTORY}/diagnostics" ABSOLUTE)
get_filename_component(TT_UTILS_DIRECTORY "${TT_ROOT_DIRECTORY}/utils/src" ABSOLUTE)
get_filename_component(TT_TEXTBOX_SRC_DIRECTORY "." ABSOLUTE)
configure_file("${TT_TEXTBOX_SRC_DIRECTORY}/TTConfig.hpp.in" TTConfig.hpp)
set(TT_DIAGNOSTICS_LIB "tteams-diagnostics")
set(TT_UTILS_LIB "tteams-utils")
set(TT_TEXTBOX_LIB "tteams-textbox")
set(TT_TEXTBOX_EXE "tteams-textbox-exe")
set(TT_TEXTBOX_HANDLER_LIB "tteams-textbox-handler")
set(TT_TEXTBOX_SCRIPTS "${TT_TEXTBOX_SRC_DIRECTORY}/tteams-textbox.sh")
foreach(TT_TEXTBOX_SCRIPT ${TT_TEXTBOX_SCRIPTS})
  configure_file("${TT_TEXTBOX_SCRIPT}" "${PROJECT_BINARY_DIR}" COPYONLY)
endforeach()

# Resolve dependencies
find_package(Threads REQUIRED)
if (NOT TARGET ${TT_DIAGNOSTICS_LIB})
  add_subdirectory("${TT_DIAGNOSTICS_DIRECTORY}" "${TT_DIAGNOSTICS_LIB}")
endif()
if (NOT TARGET ${TT_UTILS_LIB})
  add_subdirectory("${TT_UTILS_DIRECTORY}" "${TT_UTILS_LIB}")
endif()

# Build library
add_library(${TT_TEXTBOX_LIB}
  "${TT_TEXTBOX_SRC_DIRECTORY}/TTTextBoxSettings.cpp"
  "${TT_TEXTBOX_SRC_DIRECTORY}/TTTextBox.cpp"
)
set_target_properties(${TT_TEXTBOX_LIB} PROPERTIES VERSION ${PROJECT_VERSION})
target_include_directories(${TT_TEXTBOX_LIB} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${TT_TEXTBOX_LIB} PUBLIC "${TT_TEXTBOX_SRC_DIRECTORY}")
target_include_directories (${TT_TEXTBOX_LIB} PUBLIC $<TARGET_PROPERTY:${TT_DIAGNOSTICS_LIB},INTERFACE_INCLUDE_DIRECTORIES>)
target_include_directories (${TT_TEXTBOX_LIB} PUBLIC $<TARGET_PROPERTY:${TT_UTILS_LIB},INTERFACE_INCLUDE_DIRECTORIES>)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  target_link_libraries(${TT_TEXTBOX_LIB} ${TT_DIAGNOSTICS_LIB})
endif()
target_link_libraries(${TT_TEXTBOX_LIB} ${TT_UTILS_LIB})

# Build executable
add_executable(${TT_TEXTBOX_EXE}
  "${TT_TEXTBOX_SRC_DIRECTORY}/Main.cpp"
)
set_target_properties(${TT_TEXTBOX_EXE} PROPERTIES OUTPUT_NAME ${TT_TEXTBOX_LIB})
target_include_directories(${TT_TEXTBOX_EXE} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${TT_TEXTBOX_EXE} PUBLIC "${TT_TEXTBOX_SRC_DIRECTORY}")
target_include_directories (${TT_TEXTBOX_EXE} PUBLIC $<TARGET_PROPERTY:${TT_DIAGNOSTICS_LIB},INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(${TT_TEXTBOX_EXE} ${TT_TEXTBOX_LIB})

# Build library
add_library(${TT_TEXTBOX_HANDLER_LIB}
  "${TT_TEXTBOX_SRC_DIRECTORY}/TTTextBoxSettings.cpp"
  "${TT_TEXTBOX_SRC_DIRECTORY}/TTTextBoxHandler.cpp"
)
target_include_directories(${TT_TEXTBOX_HANDLER_LIB} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${TT_TEXTBOX_HANDLER_LIB} PUBLIC "${TT_TEXTBOX_SRC_DIRECTORY}")
target_include_directories (${TT_TEXTBOX_HANDLER_LIB} PUBLIC $<TARGET_PROPERTY:${TT_DIAGNOSTICS_LIB},INTERFACE_INCLUDE_DIRECTORIES>)
target_include_directories (${TT_TEXTBOX_HANDLER_LIB} PUBLIC $<TARGET_PROPERTY:${TT_UTILS_LIB},INTERFACE_INCLUDE_DIRECTORIES>)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  target_link_libraries(${TT_TEXTBOX_HANDLER_LIB} ${TT_DIAGNOSTICS_LIB})
endif()
target_link_libraries(${TT_TEXTBOX_HANDLER_LIB} ${TT_UTILS_LIB})

# Installation rules
install(TARGETS ${TT_TEXTBOX_EXE} DESTINATION "bin")
install(TARGETS ${TT_TEXTBOX_LIB} DESTINATION "lib")
install(TARGETS ${TT_TEXTBOX_HANDLER_LIB} DESTINATION "lib")
install(FILES ${TT_TEXTBOX_SCRIPTS} DESTINATION "bin"
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
)
