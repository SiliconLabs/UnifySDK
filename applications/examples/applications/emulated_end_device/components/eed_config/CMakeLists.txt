# Example MQTT Device config
add_library(eed_config src/eed_config.c)

target_link_libraries(eed_config PUBLIC unify)
target_include_directories(eed_config PUBLIC include)
target_compile_definitions(
  eed_config PRIVATE CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
                     CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

if(BUILD_TESTING)
  # Unit tests
  add_subdirectory(test)

  # mocks
  target_add_mock(eed_config)
endif()
