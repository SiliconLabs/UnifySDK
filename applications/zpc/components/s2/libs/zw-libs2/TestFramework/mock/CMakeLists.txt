
# mock target may be imported be several projects, e.g. 700 series and libs2 simultaneously.
# This test ensures that the target named 'mock' will only be declared if it is not already declared.
if (NOT TARGET mock)
  add_library(mock mock_control.c)
  target_link_libraries(mock unity)
  target_include_directories(mock PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
  add_subdirectory(test)
endif (NOT TARGET mock)
