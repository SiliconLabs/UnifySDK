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
  zwave_command_classes
  ALLOW_LIST
  ".*zwave_command_class.*|zwave_firmware.*"
  BLOCK_LIST
  "zwave_controller.*"
  HEADERS
  "$<TARGET_PROPERTY:zwave_command_classes,SOURCE_DIR>/include/*.h"
  DEPENDS
  zwave_command_classes)

generate_rust_bindings(
  NAME
  zwave_command_handler
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
  zwave_controller
  ALLOW_LIST
  ".*zwave_controller.*|zwave_node_.*"
  HEADERS
  "$<TARGET_PROPERTY:zwave_controller,SOURCE_DIR>/include/*.h"
  "${PROJECT_SOURCE_DIR}/applications/zpc/components/zpc_attribute_store/include/zwave_utils.h"
  DEPENDS
  zwave_controller)

generate_rust_bindings(
  NAME
  zwave_tx
  ALLOW_LIST
  ".*zwave_tx.*|ZWAVE_TX.*"
  BLOCK_LIST
  ".*zwave_controller.*"
  HEADERS
  "$<TARGET_PROPERTY:zwave_tx,SOURCE_DIR>/include/*.h"
  DEPENDS
  zwave_tx)

cargo_build(MANIFEST_FILE rust_command_classes/Cargo.toml)
add_subdirectory(zwave_volatile_attr_poll_manager)


