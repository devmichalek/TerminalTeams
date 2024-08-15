cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsChatAutotests VERSION 1.0)

# Set literals
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
set(TT_CHAT_LIB "tteams-chat")
set(TT_CHAT_EXE "tteams-chat-exe")
set(TT_CHAT_HANDLER_LIB "tteams-chat-handler")
set(TT_CHAT_HANDLER_EXE "tteams-chat-handler-exe")
get_filename_component(TT_CHAT_HANDLER_TEST_DIRECTORY "." ABSOLUTE)
get_filename_component(TT_CHAT_DIRECTORY "../src" ABSOLUTE)
set(TT_CHAT_HANDLER_TEST_SCRIPTS
  "tteams-chat-autotests.sh"
  "tteams-chat-autotests-happy-path.sh"
  "tteams-chat-autotests-happy-path-additional-whitespaces.sh"
  "tteams-chat-autotests-happy-path-duplicated-whitespaces.sh"
  "tteams-chat-autotests-happy-path-long-words.sh"
  "tteams-chat-autotests-unhappy-path.sh"
  "tteams-chat-autotests-verdict.sh"
)
set(TT_CHAT_HANDLER_DST "autotests")

# Resolve dependencies
find_package(Threads REQUIRED)
if (NOT TARGET ${TT_CHAT_LIB})
  add_subdirectory(${TT_CHAT_DIRECTORY} ${TT_CHAT_LIB} EXCLUDE_FROM_ALL)
endif()

# Build executable
add_executable(${TT_CHAT_HANDLER_EXE}
  "${TT_CHAT_HANDLER_TEST_DIRECTORY}/TTChatHandlerTest.cpp"
)
set_target_properties(${TT_CHAT_HANDLER_EXE} PROPERTIES OUTPUT_NAME ${TT_CHAT_HANDLER_LIB})
target_include_directories(${TT_CHAT_HANDLER_EXE} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${TT_CHAT_HANDLER_EXE} PUBLIC "${TT_CHAT_DIRECTORY}")
target_link_libraries(${TT_CHAT_HANDLER_EXE}
  Threads::Threads
  ${TT_CHAT_HANDLER_LIB}
)
add_dependencies(${TT_CHAT_HANDLER_EXE} ${TT_CHAT_EXE})

# Installation rules
install(TARGETS ${TT_CHAT_HANDLER_EXE} DESTINATION "${TT_CHAT_HANDLER_DST}")
install(TARGETS ${TT_CHAT_EXE} DESTINATION "${TT_CHAT_HANDLER_DST}")
install(FILES ${TT_CHAT_HANDLER_TEST_SCRIPTS} DESTINATION "${TT_CHAT_HANDLER_DST}"
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
)
