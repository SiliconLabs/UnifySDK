
add_library(rust_interface_headers INTERFACE)
target_include_directories(rust_interface_headers INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/unify_middleware/include>)

install(
  FILES unify_middleware/include/rust_contiki_fixt.h unify_middleware/include/rust_attribute_store.h 
  DESTINATION include/uic
  COMPONENT libuic-dev)
