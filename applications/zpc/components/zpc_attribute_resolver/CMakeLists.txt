add_library(
  zpc_attribute_resolver
  src/zpc_attribute_resolver.c src/zpc_attribute_resolver_callbacks.cpp
  src/zpc_attribute_resolver_group.cpp src/zpc_attribute_resolver_send.c)

target_include_directories(
  zpc_attribute_resolver
  PUBLIC include
  PRIVATE src
)

target_link_libraries(
  zpc_attribute_resolver
  PUBLIC
    uic_attribute_resolver
  PRIVATE
    zpc_attribute_store
    uic_log
    zwave_tx_groups
    uic_attribute_store
    zpc_utils
    zwave_command_classes
)

install(TARGETS zpc_attribute_resolver LIBRARY DESTINATION lib)

add_subdirectory(test)

target_add_mock(zpc_attribute_resolver)
