add_library(zwave_tx_scheme_selector src/zwave_tx_scheme_selector.c)

target_include_directories(
  zwave_tx_scheme_selector
  PUBLIC include
)

target_link_libraries(
  zwave_tx_scheme_selector
  PUBLIC
  zpc_attribute_store
  PRIVATE
  zwave_controller
  zwave_tx
)
target_interface_libraries(zwave_tx_scheme_selector zwave_tx_groups)

# Mocks
target_add_mock(zwave_tx_scheme_selector)

add_subdirectory(test)
