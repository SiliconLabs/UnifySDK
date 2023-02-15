# ZPC ApplicationMonitoring library
add_library(zpc_application_monitoring src/zpc_application_monitoring.c)
target_link_libraries(zpc_application_monitoring PRIVATE unify)
target_include_directories(zpc_application_monitoring PUBLIC include)

if(BUILD_TESTING)
  # mocks
  target_add_mock(zpc_application_monitoring)
  # Test
  add_subdirectory(test)
endif()
