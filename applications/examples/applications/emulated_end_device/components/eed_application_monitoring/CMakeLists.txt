# EED ApplicationMonitoring library
add_library(eed_application_monitoring src/eed_application_monitoring.c)
target_link_libraries(eed_application_monitoring PUBLIC unify)
target_include_directories(eed_application_monitoring PUBLIC include)

if(BUILD_TESTING)
    # mocks
    target_add_mock(eed_application_monitoring)

    # Test
    add_subdirectory(test)
endif()