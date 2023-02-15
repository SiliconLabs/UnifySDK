# ZigPC ApplicationMonitoring library
add_library(zigpc_application_monitoring src/zigpc_application_monitoring.c)
target_link_libraries(zigpc_application_monitoring
                      PRIVATE unify_application_monitoring)
target_include_directories(zigpc_application_monitoring PUBLIC include)

if(BUILD_TESTING)
  # mocks
  target_add_mock(zigpc_application_monitoring)

  add_library(zigpc_application_monitoring_shared SHARED
              src/zigpc_application_monitoring.c)
  target_link_libraries(zigpc_application_monitoring_shared unify)
  # Test
  add_subdirectory(test)
endif()
