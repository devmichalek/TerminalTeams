cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsHandler VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(Threads REQUIRED)

set(TT_CHAT tteams-chat)
set(TT_CHAT_HANDLER tteams-chat-handler)
set(TT_CHAT_EXE tteams-chat-exe)
set(TT_CHAT_HANDLER_TEST tteams-chat-handler-test)

get_filename_component(TT_CHAT_HANDLER_TEST_DIRECTORY "." ABSOLUTE)
get_filename_component(TT_CHAT_DIRECTORY "../src" ABSOLUTE)

set(TT_CHAT_HANDLER_TEST_SCRIPTS
  "tteams-chat-autotests.sh"
  "tteams-chat-autotests-happy-path.sh"
)

if (NOT TARGET ${TT_CHAT})
  add_subdirectory(${TT_CHAT_DIRECTORY} ${TT_CHAT} EXCLUDE_FROM_ALL)
endif()

add_executable(${TT_CHAT_HANDLER_TEST}
  "${TT_CHAT_HANDLER_TEST_DIRECTORY}/TTChatHandlerTest.cpp"
)
set_target_properties(${TT_CHAT_HANDLER_TEST} PROPERTIES OUTPUT_NAME ${TT_CHAT_HANDLER})
target_include_directories(${TT_CHAT_HANDLER_TEST} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${TT_CHAT_HANDLER_TEST} PUBLIC "${TT_CHAT_DIRECTORY}")
target_link_libraries(${TT_CHAT_HANDLER_TEST}
  Threads::Threads
  ${TT_CHAT_HANDLER}
)
add_dependencies(${TT_CHAT_HANDLER_TEST} ${TT_CHAT_EXE})

set(TT_CHAT_HANDLER_DST "autotests")
install(TARGETS ${TT_CHAT_HANDLER_TEST} DESTINATION "${TT_CHAT_HANDLER_DST}")
install(TARGETS ${TT_CHAT_EXE} DESTINATION "${TT_CHAT_HANDLER_DST}")
install(FILES ${TT_CHAT_HANDLER_TEST_SCRIPTS} DESTINATION "${TT_CHAT_HANDLER_DST}"
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
)
