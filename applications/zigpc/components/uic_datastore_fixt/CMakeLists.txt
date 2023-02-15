# Unify datastore fixtures for ZigPC
add_library(zigpc_uic_datastore_fixt src/zigpc_uic_datastore_fixt.c)
target_link_libraries(zigpc_uic_datastore_fixt unify zigpc_config)
target_include_directories(zigpc_uic_datastore_fixt PUBLIC include)

if(BUILD_TESTING)
  # mocks
  target_add_mock(zigpc_uic_datastore_fixt)

  # Test
  add_subdirectory(test)
endif()
