
# Source/Include
set(zpc_config_SRC src/zpc_config.c platform/${COMPATIBLE_PLATFORM}/device_id.c)
set(zpc_config_INCLUDE include/)
# Lib
add_library(zpc_config ${zpc_config_SRC})
target_link_libraries(zpc_config uic_log uic_config)
target_include_directories(zpc_config PUBLIC ${zpc_config_INCLUDE})
target_compile_definitions(
  zpc_config PRIVATE CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}" CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

# Testing
target_add_unittest(zpc_config SOURCES test/zpc_config_test.c )

# mocks
target_add_mock(zpc_config)
