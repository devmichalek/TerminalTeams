cmake_minimum_required(VERSION 3.22)
project(TerminalTeams VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(ABSL_ENABLE_INSTALL ON)

include(FetchContent)
FetchContent_Declare(
  grpc
  GIT_REPOSITORY https://github.com/grpc/grpc.git
  GIT_TAG        v1.61.0)
#FetchContent_MakeAvailable(grpc)
# Hack to make sure fetched content install targets are excluded
FetchContent_GetProperties(grpc)
if(NOT grpc_POPULATED)
  FetchContent_Populate(grpc)
  add_subdirectory(${grpc_SOURCE_DIR} ${grpc_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# Set generated sources paths and add command
get_filename_component(TT_PROTO_FULLPATH "interface/TerminalTeams.proto" ABSOLUTE)
get_filename_component(TT_PROTO_DIRECTORY "${TT_PROTO_FULLPATH}" DIRECTORY)
set(TT_PROTO_SRCS "${CMAKE_CURRENT_BINARY_DIR}/TerminalTeams.pb.cc")
set(TT_PROTO_HDRS "${CMAKE_CURRENT_BINARY_DIR}/TerminalTeams.pb.h")
set(TT_GRPC_SRCS "${CMAKE_CURRENT_BINARY_DIR}/TerminalTeams.grpc.pb.cc")
set(TT_GRPC_HDRS "${CMAKE_CURRENT_BINARY_DIR}/TerminalTeams.grpc.pb.h")

# Add command to generate sources
add_custom_command(
      OUTPUT "${TT_PROTO_SRCS}" "${TT_PROTO_HDRS}" "${TT_GRPC_SRCS}" "${TT_GRPC_HDRS}"
      COMMAND $<TARGET_FILE:protoc>
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${TT_PROTO_DIRECTORY}"
        --plugin=protoc-gen-grpc="$<TARGET_FILE:grpc_cpp_plugin>"
        "${TT_PROTO_FULLPATH}"
      DEPENDS "${TT_PROTO_FULLPATH}")

# Create grpc & proto library
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
add_library(tt_grpc_proto
  ${TT_GRPC_SRCS}
  ${TT_GRPC_HDRS}
  ${TT_PROTO_SRCS}
  ${TT_PROTO_HDRS})
target_link_libraries(tt_grpc_proto
  grpc++_reflection
  grpc++
  libprotobuf)

# Directories
get_filename_component(TT_SCRIPTS_DIRECTORY "scripts" ABSOLUTE)
get_filename_component(TT_COMMON_DIRECTORY "common" ABSOLUTE)
get_filename_component(TT_CHAT_DIRECTORY "chat" ABSOLUTE)

# Copy config file and scripts
configure_file(interface/TTConfig.hpp.in TTConfig.hpp)
set(TT_SCRIPTS
  "${TT_SCRIPTS_DIRECTORY}/tteams.sh"
  "${TT_SCRIPTS_DIRECTORY}/tteams-engine.sh"
  "${TT_SCRIPTS_DIRECTORY}/tteams-chat.sh"
  "${TT_SCRIPTS_DIRECTORY}/tteams-contacts.sh"
  "${TT_SCRIPTS_DIRECTORY}/tteams-textbox.sh")
foreach(TT_SCRIPT ${TT_SCRIPTS})
  configure_file("${TT_SCRIPT}" "${PROJECT_BINARY_DIR}" COPYONLY)
endforeach()

# Terminal Teams Textbox + Client
set(TT_TEXTBOX tteams-textbox)
add_executable(${TT_TEXTBOX} textbox/TTClient.cpp)
target_include_directories(${TT_TEXTBOX} PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(${TT_TEXTBOX}
  tt_grpc_proto
  absl::flags
  absl::flags_parse
  grpc++_reflection
  grpc++
  libprotobuf)

# Terminal Teams Chat + Server
set(TT_CHAT tteams-chat)
set(TT_CHAT_FILES
  "${TT_COMMON_DIRECTORY}/TTEmulator.cpp"
  "${TT_CHAT_DIRECTORY}/TTChatSettings.cpp"
  "${TT_CHAT_DIRECTORY}/Main.cpp"
  "${TT_CHAT_DIRECTORY}/TTChat.cpp")
add_executable(${TT_CHAT} ${TT_CHAT_FILES})
target_include_directories(${TT_CHAT} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${TT_CHAT} PUBLIC "${TT_COMMON_DIRECTORY}")
target_link_libraries(${TT_CHAT}
  tt_grpc_proto
  absl::flags
  absl::flags_parse
  grpc++_reflection
  grpc++
  libprotobuf)

# Terminal Teams Contacts
set(TT_CONTACTS tteams-contacts)
add_executable(${TT_CONTACTS} contacts/TTContacts.cpp)
target_include_directories(${TT_CONTACTS} PUBLIC "${PROJECT_BINARY_DIR}")

install(TARGETS ${TT_TEXTBOX} DESTINATION .)
install(TARGETS ${TT_CHAT} DESTINATION .)
install(TARGETS ${TT_CONTACTS} DESTINATION .)
install(FILES ${TT_SCRIPTS} DESTINATION .
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ)