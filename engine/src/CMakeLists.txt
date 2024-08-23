cmake_minimum_required(VERSION 3.22)
project(TerminalTeams VERSION 1.0)

# Set literals
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(ABSL_ENABLE_INSTALL ON)
set(EXT_ABSL_FLAGS_LIB "absl::flags")
set(EXT_ABSL_FLAGS_PARSE_LIB "absl::flags_parse")
set(EXT_GRPC_REFLECTION_LIB "grpc++_reflection")
set(EXT_GRPC_LIB "grpc++")
set(EXT_PROTOBUF_LIB "libprotobuf")
set(TT_GRPC_LIB "tteams-grpc")
set(TT_ENGINE_EXE "tteams-engine-exe")
set(TT_ENGINE_LIB "tteams-engine")
set(TT_DIAGNOSTICS_LIB "tteams-diagnostics")
set(TT_UTILS_LIB "tteams-utils")
set(TT_CHAT_LIB "tteams-chat")
set(TT_CONTACTS_LIB "tteams-contacts")
set(TT_TEXTBOX_LIB "tteams-textbox")
set(TT_CHAT_HANDLER_LIB "tteams-chat-handler")
set(TT_CONTACTS_HANDLER_LIB "tteams-contacts-handler")
set(TT_TEXTBOX_HANDLER_LIB "tteams-textbox-handler")
get_filename_component(TT_ROOT_DIRECTORY "../../" ABSOLUTE)
get_filename_component(TT_DIAGNOSTICS_DIRECTORY "${TT_ROOT_DIRECTORY}/diagnostics" ABSOLUTE)
get_filename_component(TT_UTILS_DIRECTORY "${TT_ROOT_DIRECTORY}/utils/src" ABSOLUTE)
get_filename_component(TT_ENGINE_SRC_DIRECTORY "." ABSOLUTE)
get_filename_component(TT_PROTO_FULLPATH "${TT_ENGINE_SRC_DIRECTORY}/TerminalTeams.proto" ABSOLUTE)
get_filename_component(TT_PROTO_DIRECTORY "${TT_PROTO_FULLPATH}" DIRECTORY)
get_filename_component(TT_CHAT_DIRECTORY "${TT_ROOT_DIRECTORY}/chat/src" ABSOLUTE)
get_filename_component(TT_CONTACTS_DIRECTORY "${TT_ROOT_DIRECTORY}/contacts/src" ABSOLUTE)
get_filename_component(TT_TEXTBOX_DIRECTORY "${TT_ROOT_DIRECTORY}/textbox/src" ABSOLUTE)
configure_file("${TT_ENGINE_SRC_DIRECTORY}/TTConfig.hpp.in" TTConfig.hpp)
set(TT_ENGINE_SCRIPTS
  "${TT_ENGINE_SRC_DIRECTORY}/tteams.sh"
  "${TT_ENGINE_SRC_DIRECTORY}/tteams-engine.sh"
)
foreach(TT_SCRIPT ${TT_ENGINE_SCRIPTS})
  configure_file("${TT_SCRIPT}" "${PROJECT_BINARY_DIR}" COPYONLY)
endforeach()

# Resolve dependencies
find_package(Threads REQUIRED)
if (NOT TARGET ${TT_DIAGNOSTICS_LIB})
  add_subdirectory("${TT_DIAGNOSTICS_DIRECTORY}" "${TT_DIAGNOSTICS_LIB}")
endif()
if (NOT TARGET ${TT_UTILS_LIB})
  add_subdirectory("${TT_UTILS_DIRECTORY}" "${TT_UTILS_LIB}")
endif()
if (NOT TARGET ${TT_CHAT_LIB})
  add_subdirectory("${TT_CHAT_DIRECTORY}" "${TT_CHAT_LIB}" EXCLUDE_FROM_ALL)
endif()
if (NOT TARGET ${TT_CONTACTS_LIB})
  add_subdirectory("${TT_CONTACTS_DIRECTORY}" "${TT_CONTACTS_LIB}" EXCLUDE_FROM_ALL)
endif()
if (NOT TARGET ${TT_TEXTBOX_LIB})
  add_subdirectory("${TT_TEXTBOX_DIRECTORY}" "${TT_TEXTBOX_LIB}" EXCLUDE_FROM_ALL)
endif()
include(FetchContent)
FetchContent_Declare(
  grpc
  GIT_REPOSITORY https://github.com/grpc/grpc.git
  GIT_TAG        v1.61.0)
FetchContent_GetProperties(grpc)
if(NOT grpc_POPULATED)
  FetchContent_Populate(grpc)
  add_subdirectory(${grpc_SOURCE_DIR} ${grpc_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# Set generated sources paths and add command
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
      DEPENDS "${TT_PROTO_FULLPATH}"
)

# Build library
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
add_library(${TT_GRPC_LIB}
  ${TT_GRPC_SRCS}
  ${TT_GRPC_HDRS}
  ${TT_PROTO_SRCS}
  ${TT_PROTO_HDRS}
)
target_link_libraries(${TT_GRPC_LIB}
  ${EXT_GRPC_REFLECTION_LIB}
  ${EXT_GRPC_LIB}
  ${EXT_PROTOBUF_LIB}
)

# Build library
add_library(${TT_ENGINE_LIB}
  "${TT_ENGINE_SRC_DIRECTORY}/TTEngine.cpp"
  "${TT_ENGINE_SRC_DIRECTORY}/TTEngineSettings.cpp"
  "${TT_ENGINE_SRC_DIRECTORY}/TTBroadcasterChat.cpp"
  "${TT_ENGINE_SRC_DIRECTORY}/TTBroadcasterDiscovery.cpp"
  "${TT_ENGINE_SRC_DIRECTORY}/TTNeighborsServiceChat.cpp"
  "${TT_ENGINE_SRC_DIRECTORY}/TTNeighborsServiceDiscovery.cpp"
)
target_include_directories (${TT_ENGINE_LIB} PUBLIC $<TARGET_PROPERTY:${TT_DIAGNOSTICS_LIB},INTERFACE_INCLUDE_DIRECTORIES>)
target_include_directories (${TT_ENGINE_LIB} PUBLIC $<TARGET_PROPERTY:${TT_UTILS_LIB},INTERFACE_INCLUDE_DIRECTORIES>)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  target_link_libraries(${TT_ENGINE_LIB} ${TT_DIAGNOSTICS_LIB})
endif()
target_link_libraries(${TT_ENGINE_LIB}
  ${TT_GRPC_LIB}
  ${EXT_ABSL_FLAGS_LIB}
  ${EXT_ABSL_FLAGS_PARSE_LIB}
  ${EXT_GRPC_REFLECTION_LIB}
  ${EXT_GRPC_LIB}
  ${EXT_PROTOBUF_LIB}
  ${TT_CHAT_HANDLER_LIB}
  ${TT_CONTACTS_HANDLER_LIB}
  ${TT_TEXTBOX_HANDLER_LIB}
)

# Build executable
add_executable(${TT_ENGINE_EXE}
  "${TT_ENGINE_SRC_DIRECTORY}/Main.cpp"
)
target_include_directories(${TT_ENGINE_EXE} PUBLIC "${PROJECT_BINARY_DIR}")
set_target_properties(${TT_ENGINE_EXE} PROPERTIES OUTPUT_NAME ${TT_ENGINE_LIB})
target_link_libraries(${TT_ENGINE_EXE} ${TT_ENGINE_LIB})

# Installation rules
install(TARGETS ${TT_ENGINE_EXE} DESTINATION "bin")
install(TARGETS ${TT_GRPC_LIB} DESTINATION "lib")
install(FILES ${TT_ENGINE_SCRIPTS} DESTINATION "bin"
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
)
