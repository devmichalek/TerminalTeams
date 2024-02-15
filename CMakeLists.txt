cmake_minimum_required(VERSION 3.22)
project(TerminalTeams VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(ABSL_ENABLE_INSTALL ON)

include(FetchContent)
FetchContent_Declare(
  grpc
  GIT_REPOSITORY https://github.com/grpc/grpc.git
  GIT_TAG        v1.61.0)
FetchContent_MakeAvailable(grpc)

# Set generated sources paths and add command
get_filename_component(tt_proto "interface/TerminalTeams.proto" ABSOLUTE)
get_filename_component(tt_proto_path "${tt_proto}" PATH)
set(tt_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/TerminalTeams.pb.cc")
set(tt_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/TerminalTeams.pb.h")
set(tt_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/TerminalTeams.grpc.pb.cc")
set(tt_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/TerminalTeams.grpc.pb.h")

# Add command to generate sources
add_custom_command(
      OUTPUT "${tt_proto_srcs}" "${tt_proto_hdrs}" "${tt_grpc_srcs}" "${tt_grpc_hdrs}"
      COMMAND $<TARGET_FILE:protoc>
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${tt_proto_path}"
        --plugin=protoc-gen-grpc="$<TARGET_FILE:grpc_cpp_plugin>"
        "${tt_proto}"
      DEPENDS "${tt_proto}")

# Create grpc & proto library
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
add_library(tt_grpc_proto
  ${tt_grpc_srcs}
  ${tt_grpc_hdrs}
  ${tt_proto_srcs}
  ${tt_proto_hdrs})
target_link_libraries(tt_grpc_proto
  grpc++_reflection
  grpc++
  libprotobuf)

# Config file
configure_file(interface/Config.hpp.in Config.hpp)

# Client
add_executable(ttclient client/TTClient.cpp)
target_include_directories(ttclient PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(ttclient
  tt_grpc_proto
  absl::flags
  absl::flags_parse
  grpc++_reflection
  grpc++
  libprotobuf)

# Server
add_executable(ttserver server/TTServer.cpp)
target_include_directories(ttserver PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(ttserver
  tt_grpc_proto
  absl::flags
  absl::flags_parse
  grpc++_reflection
  grpc++
  libprotobuf)
