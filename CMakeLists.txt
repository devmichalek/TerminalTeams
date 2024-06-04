cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsBundle VERSION 1.0)

add_subdirectory("./chat")
add_subdirectory("./contacts")
add_subdirectory("./textbox")
add_subdirectory("./engine")

set(TT_AUTOTEST_SCRIPTS
    "autotests/tteams-autotests.sh"
)

set(TT_UNITTEST_SCRIPTS
    "unittests/tteams-unittests.sh"
)

install(FILES ${TT_AUTOTEST_SCRIPTS} DESTINATION "autotests"
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
)

install(FILES ${TT_UNITTEST_SCRIPTS} DESTINATION "unittests"
  PERMISSIONS
    OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
)
