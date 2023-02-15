# EPC datastore library
add_library(epc_datastore src/epc_datastore.c)
target_link_libraries(epc_datastore PRIVATE unify epc_config)
target_include_directories(epc_datastore PUBLIC include)

if(BUILD_TESTING)
  # mocks
  target_add_mock(epc_datastore)

  # Test
  add_subdirectory(test)
endif()
