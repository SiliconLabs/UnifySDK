add_library(unify_application_monitoring OBJECT
            src/unify_application_monitoring.cpp)

target_include_directories(
  unify_application_monitoring
  PRIVATE src
  PUBLIC include)

target_link_libraries(unify_application_monitoring PRIVATE uic_mqtt
                                                           uic_dotdot_mqtt)

install_headers(unify_application_monitoring uic libuic-dev)

if(BUILD_TESTING)
  # Mocks
  target_add_mock(unify_application_monitoring)
  # Unit tests
  add_subdirectory(test)
endif()
