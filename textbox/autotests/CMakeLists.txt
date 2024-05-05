cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsHandler VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(Threads REQUIRED)

set(TT_TEXTBOX tteams-textbox)
set(TT_TEXTBOX_HANDLER tteams-textbox-handler)
set(TT_TEXTBOX_EXE tteams-textbox-exe)
set(TT_TEXTBOX_HANDLER_TEST tteams-textbox-handler-test)

get_filename_component(TT_TEXTBOX_HANDLER_TEST_DIRECTORY "." ABSOLUTE)
get_filename_component(TT_TEXTBOX_DIRECTORY "../src" ABSOLUTE)

set(TT_TEXTBOX_HANDLER_TEST_SCRIPTS
  "tteams-textbox-autotests.sh"
  "tteams-textbox-autotests-happy-path.sh"
)

add_subdirectory(${TT_TEXTBOX_DIRECTORY} ${TT_TEXTBOX} EXCLUDE_FROM_ALL)

add_executable(${TT_TEXTBOX_HANDLER_TEST}
  "${TT_TEXTBOX_HANDLER_TEST_DIRECTORY}/TTTextBoxHandlerTest.cpp")
  set_target_properties(${TT_TEXTBOX_HANDLER_TEST} PROPERTIES OUTPUT_NAME ${TT_TEXTBOX_HANDLER})
target_include_directories(${TT_TEXTBOX_HANDLER_TEST} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${TT_TEXTBOX_HANDLER_TEST} PUBLIC "${TT_TEXTBOX_DIRECTORY}")
target_link_libraries(${TT_TEXTBOX_HANDLER_TEST}
  Threads::Threads
  ${TT_TEXTBOX_HANDLER})
add_dependencies(${TT_TEXTBOX_HANDLER_TEST} ${TT_TEXTBOX_EXE})

install(TARGETS ${TT_TEXTBOX_HANDLER_TEST} ${TT_TEXTBOX_EXE} DESTINATION .)
install(FILES ${TT_TEXTBOX_HANDLER_TEST_SCRIPTS} DESTINATION .
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ)