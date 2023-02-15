# AoXPC ApplicationMonitoring library
add_library(aoxpc_application_monitoring src/aoxpc_application_monitoring.c)
target_link_libraries(aoxpc_application_monitoring PRIVATE unify_application_monitoring)
target_include_directories(aoxpc_application_monitoring PUBLIC include)

if(BUILD_TESTING)
    # mocks & Test
    target_add_mock(aoxpc_application_monitoring)

    # Test
    add_subdirectory(test)
endif()