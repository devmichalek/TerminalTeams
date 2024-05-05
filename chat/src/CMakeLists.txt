cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsChat VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(Threads REQUIRED)

get_filename_component(TT_ROOT_DIRECTORY "../../" ABSOLUTE)
get_filename_component(TT_DIAGNOSTICS_DIRECTORY "${TT_ROOT_DIRECTORY}/diagnostics" ABSOLUTE)
get_filename_component(TT_CHAT_SRC_DIRECTORY "." ABSOLUTE)
set(TT_DIAGNOSTICS_LIB "tteams-diagnostics")

if (NOT TARGET ${TT_DIAGNOSTICS_LIB})
  add_subdirectory("${TT_DIAGNOSTICS_DIRECTORY}" "${TT_DIAGNOSTICS_LIB}")
endif()

set(TT_CHAT_SCRIPTS
  "${TT_CHAT_SRC_DIRECTORY}/tteams-chat.sh"
)
foreach(TT_CHAT_SCRIPT ${TT_CHAT_SCRIPTS})
  configure_file("${TT_CHAT_SCRIPT}" "${PROJECT_BINARY_DIR}" COPYONLY)
endforeach()

set(TT_CHAT_LIB tteams-chat)
add_library(${TT_CHAT_LIB}
  "${TT_CHAT_SRC_DIRECTORY}/TTChatSettings.cpp"
  "${TT_CHAT_SRC_DIRECTORY}/TTChat.cpp"
)
set_target_properties(${TT_CHAT_LIB} PROPERTIES VERSION ${PROJECT_VERSION})
target_include_directories(${TT_CHAT_LIB} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${TT_CHAT_LIB} PUBLIC "${TT_CHAT_SRC_DIRECTORY}")
target_include_directories (${TT_CHAT_LIB} PUBLIC $<TARGET_PROPERTY:${TT_DIAGNOSTICS_LIB},INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(${TT_CHAT_LIB} ${TT_DIAGNOSTICS_LIB})

set(TT_CHAT tteams-chat-exe)
add_executable(${TT_CHAT}
  "${TT_CHAT_SRC_DIRECTORY}/Main.cpp"
)
set_target_properties(${TT_CHAT} PROPERTIES OUTPUT_NAME ${TT_CHAT_LIB})
target_include_directories(${TT_CHAT} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${TT_CHAT} PUBLIC "${TT_CHAT_SRC_DIRECTORY}")
target_include_directories (${TT_CHAT} PUBLIC $<TARGET_PROPERTY:${TT_DIAGNOSTICS_LIB},INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(${TT_CHAT} ${TT_CHAT_LIB})

set(TT_CHAT_HANDLER_LIB tteams-chat-handler)
add_library(${TT_CHAT_HANDLER_LIB}
"${TT_CHAT_SRC_DIRECTORY}/TTChatHandler.cpp"
)
target_include_directories(${TT_CHAT_HANDLER_LIB} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${TT_CHAT_HANDLER_LIB} PUBLIC "${TT_CHAT_SRC_DIRECTORY}")
target_include_directories (${TT_CHAT_HANDLER_LIB} PUBLIC $<TARGET_PROPERTY:${TT_DIAGNOSTICS_LIB},INTERFACE_INCLUDE_DIRECTORIES>)

install(TARGETS ${TT_CHAT}  DESTINATION "bin")
install(TARGETS ${TT_CHAT_LIB} DESTINATION "lib")
install(TARGETS ${TT_CHAT_HANDLER_LIB} DESTINATION "lib")
install(FILES ${TT_CHAT_SCRIPTS} DESTINATION "bin"
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
)
