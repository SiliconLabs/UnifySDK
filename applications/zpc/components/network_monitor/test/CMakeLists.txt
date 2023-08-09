if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # "--allow-multiple-definition" can cause some weird symptons as we leave it
  # up to the linker which symbol gets merged into the test binary.
  add_link_options("-Wl,--unresolved-symbols=ignore-in-object-files"
                   "-Wl,--allow-multiple-definition")
endif()
# Network monitor test (with real attribute store)
target_add_unittest(
  network_monitor
  NAME
  zwave_network_monitor_test
  SOURCES
  zwave_network_monitor_test.c
  DEPENDS

  zwave_definitions
  uic_contiki_stub
  zpc_attribute_store_test_helper
  uic_attribute_utils_test_helper
  zpc_config_mock
  uic_mqtt_mock
  zwave_controller_mock
  zwave_network_management_mock
  zcl_cluster_servers_mock
  zwave_command_classes_mock
  EXCLUDE
  network_monitor_span_persistence.c)

# SmartStart list monitor test
target_add_unittest(
  network_monitor
  NAME
  smart_start_list_monitor_test
  SOURCES
  smart_start_list_monitor_test.cpp
  DEPENDS
  zwave_controller_mock
  zwave_network_management_mock
  uic_smartstart_management_mock
  uic_contiki_stub
  zpc_attribute_store_test_helper
  EXCLUDE
  network_monitor_span_persistence.c
  network_monitor_fixt.c
  network_monitor.cpp)

# SPAN/MPAN persistence test
target_add_unittest(
  network_monitor
  NAME
  network_monitor_span_persistence_test
  SOURCES
  network_monitor_span_persistence_test.c
  DEPENDS
  zwave_tx_groups_mock
  zwave_network_management_mock
  zwave_s2_mock
  zpc_attribute_store_test_helper
  EXCLUDE
  network_monitor.cpp
  network_monitor_fixt.c)

# Failing node monitor test
target_add_unittest(
  network_monitor
  NAME
  failing_node_monitor_test
  SOURCES
  failing_node_monitor_test.c
  DEPENDS
  zwave_controller_mock

  zwave_definitions
  uic_contiki_stub
  zpc_attribute_store_test_helper
  uic_attribute_utils_test_helper)

# Sleeping nodes keep alive test
target_add_unittest(
  network_monitor
  NAME
  keep_sleeping_nodes_alive_test
  SOURCES
  keep_sleeping_nodes_alive_test.cpp
  DEPENDS
  zpc_attribute_store_test_helper
  uic_attribute_utils_test_helper
  uic_contiki_stub
  zwave_controller_mock)

# Network Monitor Utils test
target_add_unittest(
  network_monitor
  NAME
  network_monitor_utils_test
  SOURCES
  network_monitor_utils_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  uic_contiki_stub
  uic_attribute_resolver_mock)

# Fixture test
target_add_unittest(
  network_monitor
  NAME
  failing_node_monitor_fixt_test
  SOURCES
  failing_node_monitor_fixt_test.c
  DEPENDS
  uic_contiki_stub)

target_add_unittest(network_monitor NAME container_retain_test SOURCES
                    container_retain_test.cpp)
target_link_libraries(container_retain_test
                      PRIVATE uic_smartstart_management_mock)
