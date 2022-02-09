add_library(
  zpc_attribute_resolver
  src/zpc_attribute_resolver.c src/zpc_attribute_resolver_callbacks.cpp
  src/zpc_attribute_resolver_group.cpp src/zpc_attribute_resolver_send.c)

target_include_directories(
  zpc_attribute_resolver
  PUBLIC include
  PRIVATE src
          # break cyclic dependency. references to this includes should be moved
          # to a central place.
          ../zwave_command_classes/include)

target_link_libraries(
  zpc_attribute_resolver
  uic_log
  zwave_tx_groups
  uic_attribute_store
  uic_attribute_resolver
  zpc_utils
  zpc_attribute_store)

install(TARGETS zpc_attribute_resolver LIBRARY DESTINATION lib)

add_subdirectory(test)

target_add_mock(zpc_attribute_resolver)
