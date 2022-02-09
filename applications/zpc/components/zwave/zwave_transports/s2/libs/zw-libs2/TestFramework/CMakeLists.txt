# Prevent multiple "inclusions" of TestFramework
if (NOT TARGET unity)
  message(STATUS "TestFramework from ${CMAKE_CURRENT_SOURCE_DIR} included")
  add_definitions( -DUNIT_TEST )

  set(TEST_TOOLS_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL test_tools_dir)

  find_package(PythonInterp)

  include(CMakeFunctions.cmake)

  add_subdirectory(unity)
  # Check whether the building of mock framework is disabled.
  # Building of the mock framework can be disabled for the c51 or asip simply by adding the following line to the specific project at higher level:
  # e.g.: set(DISABLE_MOCK 1)
  # Warning, Do not set DISABLE_MOCK in this file.
  if (NOT DEFINED DISABLE_MOCK)
    add_subdirectory(mock)
  endif (NOT DEFINED DISABLE_MOCK)
else()
  message(STATUS "TestFramework from ${CMAKE_CURRENT_SOURCE_DIR} NOT included")
endif()
