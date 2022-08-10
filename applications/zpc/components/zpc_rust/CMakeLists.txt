generate_rust_bindings(
  NAME
  rust_command_classes
  ALLOW_LIST
  ".*firmware_.*"
  BLOCK_LIST
  "attribute_store_node_t"
  HEADERS
  "$<TARGET_PROPERTY:zwave_command_classes,SOURCE_DIR>/src/zwave_command_class_firmware_update_internals.h"
)

generate_rust_bindings(
  NAME
  zwave_command_classes_sys
  ALLOW_LIST
  ".*zwave_command_class.*|zwave_firmware.*"
  BLOCK_LIST
  "zwave_controller.*"
  HEADERS
  "$<TARGET_PROPERTY:zwave_command_classes,SOURCE_DIR>/include/*.h"
  DEPENDS
  zwave_command_classes 
  uic_attribute_resolver
  zpc_config
  zpc_attribute_store
  zpc_attribute_store_test_helper)

generate_rust_bindings(
  NAME
  zwave_command_handler_sys
  ALLOW_LIST
  ".*zwave_command_handler.*"
  BLOCK_LIST
  "zwave_controller.*"
  HEADERS
  "$<TARGET_PROPERTY:zwave_command_handler,SOURCE_DIR>/include/*.h"
  DEPENDS
  zwave_command_handler)

generate_rust_bindings(
  NAME
  zwave_controller_sys
  ALLOW_LIST
  ".*zwave_controller.*|zwave_node_.*"
  HEADERS
  "$<TARGET_PROPERTY:zwave_controller,SOURCE_DIR>/include/*.h"
  "${PROJECT_SOURCE_DIR}/applications/zpc/components/zpc_attribute_store/include/zwave_utils.h"
  DEPENDS
  zwave_controller)

generate_rust_bindings(
  NAME
  zwave_tx_sys
  ALLOW_LIST
  ".*zwave_tx.*|ZWAVE_TX.*"
  BLOCK_LIST
  ".*zwave_controller.*"
  HEADERS
  "$<TARGET_PROPERTY:zwave_tx,SOURCE_DIR>/include/*.h"
  DEPENDS
  zwave_tx)

generate_rust_bindings(
  NAME
  zpc_attribute_store
  ALLOW_LIST
  "get_zpc_.*"
  HEADERS
  "$<TARGET_PROPERTY:zpc_attribute_store,SOURCE_DIR>/include/*.h"
  DEPENDS
  "zpc_attribute_store")

cargo_build(MANIFEST_FILE Cargo.toml)

set_target_properties(
  zpc_rust PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                             ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(zpc_rust PROPERTIES INTERFACE_LINK_LIBRARIES
                                                 "pthread;dl")

install(
  FILES zwave_poll_config.yaml
  DESTINATION share/uic
  COMPONENT uic-zpc)
