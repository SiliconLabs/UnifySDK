# Positioning ApplicationMonitoring library
add_library(positioning_application_monitoring src/positioning_application_monitoring.c)
target_link_libraries(positioning_application_monitoring PRIVATE unify_application_monitoring)
target_include_directories(positioning_application_monitoring PUBLIC include)

if(BUILD_TESTING)
    # mocks
    target_add_mock(positioning_application_monitoring)

    # Test
    add_subdirectory(test)
endif()