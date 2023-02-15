# EPC ApplicationMonitoring library
add_library(epc_application_monitoring src/epc_application_monitoring.c)
target_link_libraries(epc_application_monitoring PRIVATE unify_application_monitoring)
target_include_directories(epc_application_monitoring PUBLIC include)

if(BUILD_TESTING)
    # mocks
    target_add_mock(epc_application_monitoring)

    # Test
    add_subdirectory(test)
endif()