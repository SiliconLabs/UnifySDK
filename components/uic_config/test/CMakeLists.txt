# Unit Tests
target_add_unittest(uic_config SOURCES config_test.cpp DEPENDS unify)
target_compile_definitions(
  uic_config_test PRIVATE CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
                          CONFIG_UNITTEST)
add_executable(demo demo.c)
target_link_libraries(demo uic_config)
