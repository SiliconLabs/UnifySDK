# AoX Protocol Controller Configuration
add_library(aoxpc_config src/aoxpc_config.c)
target_include_directories(aoxpc_config PUBLIC include)
target_link_libraries(aoxpc_config PUBLIC unify)

if(BUILD_TESTING)
  # Test
  add_subdirectory(test)

  # mocks
  target_add_mock(aoxpc_config)
endif()
