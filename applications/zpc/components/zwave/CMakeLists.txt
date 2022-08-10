add_subdirectory(zwave_definitions)
add_subdirectory(zwave_controller)
add_subdirectory(zwave_network_management)
add_subdirectory(zwave_transports)
add_subdirectory(zwave_tx)
add_subdirectory(zwave_tx_groups)
add_subdirectory(zwave_rx)
add_subdirectory(zwave_security_validation)
add_subdirectory(zwave_tx_scheme_selector)

# This libarary is a collection of everyting needed to run the zwave_controller,
# note that the library zpc_utils does not really belong here, but we the
# Network management module needs it for some logging. We should consider fixing
# that.
#
# zwave_rx has not been included since it very serial api specfic We should mabe
# consider collecting zwave_rx and zwave_api into a seperate library
uic_add_shared_library(
  zwave
  zwave_transports
  zwave_tx_groups
  zwave_controller
  zwave_network_management
  zwave_security_validation
  zwave_tx
  zwave_tx_scheme_selector
  zwave_s2
  zwave_s0
  zwave_api_transport
  zwave_s2_nonce_management
  s2_controller
  s2crypto
  aes
  zwave_multi_channel
  transport_service_wrapper
  transport_service
  zwave_api
  zpc_utils)

rust_add_native_libs(zwave)
