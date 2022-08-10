find_package(Edit REQUIRED)
add_library(zpc_stdin src/zpc_stdin_command_handling.cpp src/zpc_stdin_fixt.c)

target_include_directories(
  zpc_stdin
  PUBLIC include
  PRIVATE ../uic_main/include src)

target_link_libraries(
  zpc_stdin
  PUBLIC zpc_utils
         zwave_controller
         zwave_tx_groups
         uic_attribute_resolver
         unify_attribute_poll
         zwave_command_classes
         zwave_command_handler
         ucl_mqtt
         uic_stdin
         uic_mqtt
  PRIVATE uic_ota zwave_s2)

add_subdirectory(test)
