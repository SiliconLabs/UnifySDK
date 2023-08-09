add_library(zigpc_config src/zigpc_config.c src/zigpc_config_validate.cpp)
target_include_directories(zigpc_config PUBLIC include)
target_link_libraries(zigpc_config PRIVATE unify)

if(BUILD_TESTING)
  # Testing
  add_subdirectory(test)

  # Mocks
  target_add_mock(zigpc_config)
endif()
