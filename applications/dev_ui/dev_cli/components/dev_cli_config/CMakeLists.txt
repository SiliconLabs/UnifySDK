# Source/Include
set(DEV_CLI_CONFIG_SRC src/dev_cli_config.c)
set(DEV_CLI_CONFIG_INCLUDE include/)
# Lib
add_library(dev_cli_config ${DEV_CLI_CONFIG_SRC})
target_link_libraries(dev_cli_config uic_config uic_log)
target_include_directories(dev_cli_config PUBLIC ${DEV_CLI_CONFIG_INCLUDE})
target_compile_definitions(
  dev_cli_config PRIVATE CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")

# Testing
add_subdirectory(test)

# Mocks
target_add_mock(dev_cli_config)
