cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsContacts VERSION 1.0)

# Set literals
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
get_filename_component(TT_ROOT_DIRECTORY "../../" ABSOLUTE)
get_filename_component(TT_DIAGNOSTICS_DIRECTORY "${TT_ROOT_DIRECTORY}/diagnostics/src" ABSOLUTE)
get_filename_component(TT_UTILS_DIRECTORY "${TT_ROOT_DIRECTORY}/utils/src" ABSOLUTE)
get_filename_component(TT_CONTACTS_SRC_DIRECTORY "." ABSOLUTE)
set(TT_DIAGNOSTICS_LIB "tteams-diagnostics")
set(TT_UTILS_LIB "tteams-utils")
set(TT_CONTACTS_LIB "tteams-contacts")
set(TT_CONTACTS_EXE "tteams-contacts-exe")
set(TT_CONTACTS_HANDLER_LIB "tteams-contacts-handler")
set(TT_CONTACTS_SCRIPTS "${TT_CONTACTS_SRC_DIRECTORY}/tteams-contacts.sh")
foreach(TT_CONTACTS_SCRIPT ${TT_CONTACTS_SCRIPTS})
  configure_file("${TT_CONTACTS_SCRIPT}" "${PROJECT_BINARY_DIR}" COPYONLY)
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
add_library(${TT_CONTACTS_LIB}
  "${TT_CONTACTS_SRC_DIRECTORY}/TTContactsSettings.cpp"
  "${TT_CONTACTS_SRC_DIRECTORY}/TTContacts.cpp"
)
set_target_properties(${TT_CONTACTS_LIB} PROPERTIES VERSION ${PROJECT_VERSION})
target_include_directories(${TT_CONTACTS_LIB} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${TT_CONTACTS_LIB} PUBLIC "${TT_CONTACTS_SRC_DIRECTORY}")
target_include_directories (${TT_CONTACTS_LIB} PUBLIC $<TARGET_PROPERTY:${TT_DIAGNOSTICS_LIB},INTERFACE_INCLUDE_DIRECTORIES>)
target_include_directories (${TT_CONTACTS_LIB} PUBLIC $<TARGET_PROPERTY:${TT_UTILS_LIB},INTERFACE_INCLUDE_DIRECTORIES>)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  target_link_libraries(${TT_CONTACTS_LIB} ${TT_DIAGNOSTICS_LIB})
endif()
target_link_libraries(${TT_CONTACTS_LIB} ${TT_UTILS_LIB})

# Build executable
add_executable(${TT_CONTACTS_EXE}
  "${TT_CONTACTS_SRC_DIRECTORY}/Main.cpp"
)
set_target_properties(${TT_CONTACTS_EXE} PROPERTIES OUTPUT_NAME ${TT_CONTACTS_LIB})
target_include_directories(${TT_CONTACTS_EXE} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${TT_CONTACTS_EXE} PUBLIC "${TT_CONTACTS_SRC_DIRECTORY}")
target_include_directories (${TT_CONTACTS_EXE} PUBLIC $<TARGET_PROPERTY:${TT_DIAGNOSTICS_LIB},INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(${TT_CONTACTS_EXE} ${TT_CONTACTS_LIB})

# Build library
add_library(${TT_CONTACTS_HANDLER_LIB}
  "${TT_CONTACTS_SRC_DIRECTORY}/TTContactsHandler.cpp"
)
target_include_directories(${TT_CONTACTS_HANDLER_LIB} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${TT_CONTACTS_HANDLER_LIB} PUBLIC "${TT_CONTACTS_SRC_DIRECTORY}")
target_include_directories (${TT_CONTACTS_HANDLER_LIB} PUBLIC $<TARGET_PROPERTY:${TT_DIAGNOSTICS_LIB},INTERFACE_INCLUDE_DIRECTORIES>)
target_include_directories (${TT_CONTACTS_HANDLER_LIB} PUBLIC $<TARGET_PROPERTY:${TT_UTILS_LIB},INTERFACE_INCLUDE_DIRECTORIES>)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  target_link_libraries(${TT_CONTACTS_HANDLER_LIB} ${TT_DIAGNOSTICS_LIB})
endif()
target_link_libraries(${TT_CONTACTS_HANDLER_LIB} ${TT_UTILS_LIB})

# Installation rules
install(TARGETS ${TT_CONTACTS_EXE} DESTINATION "bin")
install(TARGETS ${TT_CONTACTS_LIB} DESTINATION "lib")
install(TARGETS ${TT_CONTACTS_HANDLER_LIB} DESTINATION "lib")
install(FILES ${TT_CONTACTS_SCRIPTS} DESTINATION "bin"
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
)
