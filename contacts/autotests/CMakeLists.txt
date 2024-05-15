cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsContactsAutoTests VERSION 1.0)

# Set literals
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
set(TT_CONTACTS_LIB tteams-contacts)
set(TT_CONTACTS_EXE tteams-contacts-exe)
set(TT_CONTACTS_HANDLER_LIB tteams-contacts-handler)
set(TT_CONTACTS_HANDLER_EXE tteams-contacts-handler-exe)
get_filename_component(TT_CONTACTS_HANDLER_TEST_DIRECTORY "." ABSOLUTE)
get_filename_component(TT_CONTACTS_DIRECTORY "../src" ABSOLUTE)
set(TT_CONTACTS_HANDLER_TEST_SCRIPTS
  "tteams-contacts-autotests.sh"
  "tteams-contacts-autotests-happy-path.sh"
)
set(TT_CONTACTS_DST "autotests")

# Resolve dependencies
find_package(Threads REQUIRED)
if (NOT TARGET ${TT_CONTACTS_LIB})
  add_subdirectory(${TT_CONTACTS_DIRECTORY} ${TT_CONTACTS_LIB} EXCLUDE_FROM_ALL)
endif()

# Build executable
add_executable(${TT_CONTACTS_HANDLER_EXE}
  "${TT_CONTACTS_HANDLER_TEST_DIRECTORY}/TTContactsHandlerTest.cpp"
)
set_target_properties(${TT_CONTACTS_HANDLER_EXE} PROPERTIES OUTPUT_NAME ${TT_CONTACTS_HANDLER_LIB})
target_include_directories(${TT_CONTACTS_HANDLER_EXE} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${TT_CONTACTS_HANDLER_EXE} PUBLIC "${TT_CONTACTS_DIRECTORY}")
target_link_libraries(${TT_CONTACTS_HANDLER_EXE}
  Threads::Threads
  ${TT_CONTACTS_HANDLER_LIB}
)
add_dependencies(${TT_CONTACTS_HANDLER_EXE} ${TT_CONTACTS_EXE})

# Installation rules
install(TARGETS ${TT_CONTACTS_HANDLER_EXE} DESTINATION "${TT_CONTACTS_DST}")
install(TARGETS ${TT_CONTACTS_EXE} DESTINATION "${TT_CONTACTS_DST}")
install(FILES ${TT_CONTACTS_HANDLER_TEST_SCRIPTS} DESTINATION "${TT_CONTACTS_DST}"
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
)
