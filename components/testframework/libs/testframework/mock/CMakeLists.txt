# mock target may be imported be several projects, e.g. zw-protocol and libs2 simultaneously.
# This test ensures that the target named 'mock' will only be declared if it is not already declared.
if (NOT TARGET mock)
  add_library(mock mock_control.c)
  target_link_libraries(mock unity)
  target_compile_definitions(mock PUBLIC USE_UNITY)
  message(STATUS "Linking mock framework with unity (from libs/testFramework/cmock/vendor/unity)")
  target_include_directories(mock PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
  add_subdirectory(test)
endif (NOT TARGET mock)
