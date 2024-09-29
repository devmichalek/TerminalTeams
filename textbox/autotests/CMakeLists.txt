cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsTextBoxAutotests VERSION 1.0)

# Set literals
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
set(TT_TEXTBOX_LIB "tteams-textbox")
set(TT_TEXTBOX_EXE "tteams-textbox-exe")
set(TT_TEXTBOX_HANDLER_LIB "tteams-textbox-handler")
set(TT_TEXTBOX_HANDLER_EXE "tteams-textbox-handler-exe")
get_filename_component(TT_TEXTBOX_HANDLER_TEST_DIRECTORY "." ABSOLUTE)
get_filename_component(TT_TEXTBOX_DIRECTORY "../src" ABSOLUTE)
set(TT_TEXTBOX_HANDLER_TEST_SCRIPTS
  "tteams-textbox-autotests.sh"
  "tteams-textbox-autotests-verdict.sh"
  "tteams-textbox-autotests-happy-path.sh"
  "tteams-textbox-autotests-happy-path-help.sh"
  "tteams-textbox-autotests-happy-path-message.sh"
  "tteams-textbox-autotests-happy-path-quit.sh"
  "tteams-textbox-autotests-happy-path-select.sh"
  "tteams-textbox-autotests-unhappy-path.sh"
  "tteams-textbox-autotests-unhappy-path-help.sh"
  "tteams-textbox-autotests-unhappy-path-kill.sh"
  "tteams-textbox-autotests-unhappy-path-quit.sh"
  "tteams-textbox-autotests-unhappy-path-select.sh"
)
set(TT_TEXTBOX_HANDLER_DST "autotests")

# Resolve dependencies
find_package(Threads REQUIRED)
if (NOT TARGET ${TT_TEXTBOX_LIB})
  add_subdirectory(${TT_TEXTBOX_DIRECTORY} ${TT_TEXTBOX_LIB} EXCLUDE_FROM_ALL)
endif()

# Build executable
add_executable(${TT_TEXTBOX_HANDLER_EXE}
  "${TT_TEXTBOX_HANDLER_TEST_DIRECTORY}/TTTextBoxHandlerTest.cpp"
)
set_target_properties(${TT_TEXTBOX_HANDLER_EXE} PROPERTIES OUTPUT_NAME ${TT_TEXTBOX_HANDLER_LIB})
target_include_directories(${TT_TEXTBOX_HANDLER_EXE} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${TT_TEXTBOX_HANDLER_EXE} PUBLIC "${TT_TEXTBOX_DIRECTORY}")
target_link_libraries(${TT_TEXTBOX_HANDLER_EXE}
  Threads::Threads
  ${TT_TEXTBOX_HANDLER_LIB}
)
add_dependencies(${TT_TEXTBOX_HANDLER_EXE} ${TT_TEXTBOX_EXE})

# Installation rules
install(TARGETS ${TT_TEXTBOX_HANDLER_EXE} DESTINATION "${TT_TEXTBOX_HANDLER_DST}")
install(TARGETS ${TT_TEXTBOX_EXE} DESTINATION "${TT_TEXTBOX_HANDLER_DST}")
install(FILES ${TT_TEXTBOX_HANDLER_TEST_SCRIPTS} DESTINATION "${TT_TEXTBOX_HANDLER_DST}"
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
)
