# EED datastore library
add_library(eed_datastore src/eed_datastore.c)
target_link_libraries(eed_datastore PUBLIC unify PRIVATE eed_config)
target_include_directories(eed_datastore PUBLIC include)

if(BUILD_TESTING)
  # mocks
  target_add_mock(eed_datastore)

  # Test
  add_subdirectory(test)
endif()
