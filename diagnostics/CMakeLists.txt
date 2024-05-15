cmake_minimum_required(VERSION 3.22)
project(TerminalTeamsDiagnosticsBundle VERSION 1.0)

set(TT_DIAGNOSTICS_LIB "tteams-diagnostics")

if (NOT TARGET ${TT_DIAGNOSTICS_LIB})
  add_subdirectory("./src" "${TT_DIAGNOSTICS_LIB}")
endif()