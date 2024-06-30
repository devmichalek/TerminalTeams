cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsEngineAutotests VERSION 1.0)

# Set literals
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
set(TT_ENGINE_PROJECT "tteams-engine")
set(TT_CHAT_PROJECT "tteams-chat")
set(TT_CONTACTS_PROJECT "tteams-contacts")
set(TT_TEXTBOX_PROJECT "tteams-textbox")
set(TT_ENGINE_EXE "tteams-engine-exe")
set(TT_CHAT_EXE "tteams-chat-exe")
set(TT_CONTACTS_EXE "tteams-contacts-exe")
set(TT_TEXTBOX_EXE "tteams-textbox-exe")
get_filename_component(TT_ROOT_DIRECTORY "../../" ABSOLUTE)
get_filename_component(TT_ENGINE_DIRECTORY "${TT_ROOT_DIRECTORY}/engine/src" ABSOLUTE)
get_filename_component(TT_CHAT_DIRECTORY "${TT_ROOT_DIRECTORY}/chat/src" ABSOLUTE)
get_filename_component(TT_CONTACTS_DIRECTORY "${TT_ROOT_DIRECTORY}/contacts/src" ABSOLUTE)
get_filename_component(TT_TEXTBOX_DIRECTORY "${TT_ROOT_DIRECTORY}/textbox/src" ABSOLUTE)
get_filename_component(TT_ENGINE_TEST_DIRECTORY "." ABSOLUTE)
set(TT_ENGINE_HANDLER_TEST_SCRIPTS
  "tteams-engine-autotests.sh"
  "tteams-engine-autotests-happy-path.sh"
  "tteams-engine-autotests-dummy-neighbor.py"
)
set(TT_ENGINE_AUTOTESTS_DST "autotests")

# Resolve dependencies
find_package(Threads REQUIRED)
if (NOT TARGET ${TT_ENGINE_PROJECT})
  add_subdirectory(${TT_ENGINE_DIRECTORY} ${TT_ENGINE_PROJECT} EXCLUDE_FROM_ALL)
endif()
if (NOT TARGET ${TT_CHAT_PROJECT})
  add_subdirectory(${TT_CHAT_DIRECTORY} ${TT_CHAT_PROJECT} EXCLUDE_FROM_ALL)
endif()
if (NOT TARGET ${TT_CONTACTS_PROJECT})
  add_subdirectory(${TT_CONTACTS_DIRECTORY} ${TT_CONTACTS_PROJECT} EXCLUDE_FROM_ALL)
endif()
if (NOT TARGET ${TT_TEXTBOX_PROJECT})
  add_subdirectory(${TT_TEXTBOX_DIRECTORY} ${TT_TEXTBOX_PROJECT} EXCLUDE_FROM_ALL)
endif()

# Generate python proto files command
set(PYTHON_PROTO_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/TerminalTeams_pb2.py
  ${CMAKE_CURRENT_BINARY_DIR}/TerminalTeams_pb2.pyi
  ${CMAKE_CURRENT_BINARY_DIR}/TerminalTeams_pb2_grpc.py
)
add_custom_command (
  OUTPUT ${PYTHON_PROTO_FILES}
  COMMAND "${TT_ENGINE_TEST_DIRECTORY}/tteams-engine-autotests-generate-proto.sh" TerminalTeams.proto ${CMAKE_CURRENT_BINARY_DIR} ${TT_ENGINE_DIRECTORY}
)
add_custom_target(PYTHON_PROTO_EPHEMERAL_TARGET ALL
  DEPENDS ${PYTHON_PROTO_FILES}
)

# Build custom target
add_custom_target(ENGINE_EPHEMERAL_TARGET ALL)
add_dependencies(ENGINE_EPHEMERAL_TARGET
  ${TT_ENGINE_EXE}
  ${TT_CHAT_EXE}
  ${TT_CONTACTS_EXE}
  ${TT_TEXTBOX_EXE}
)

# Installation rules
install(PROGRAMS ${PYTHON_PROTO_FILES} DESTINATION "${TT_ENGINE_AUTOTESTS_DST}")
install(TARGETS ${TT_ENGINE_EXE} DESTINATION "${TT_ENGINE_AUTOTESTS_DST}")
install(TARGETS ${TT_CHAT_EXE} DESTINATION "${TT_ENGINE_AUTOTESTS_DST}")
install(TARGETS ${TT_CONTACTS_EXE} DESTINATION "${TT_ENGINE_AUTOTESTS_DST}")
install(TARGETS ${TT_TEXTBOX_EXE} DESTINATION "${TT_ENGINE_AUTOTESTS_DST}")
install(FILES ${TT_ENGINE_HANDLER_TEST_SCRIPTS} DESTINATION "${TT_ENGINE_AUTOTESTS_DST}"
 PERMISSIONS
   OWNER_EXECUTE OWNER_WRITE OWNER_READ
   GROUP_EXECUTE GROUP_READ
   WORLD_EXECUTE WORLD_READ
)
