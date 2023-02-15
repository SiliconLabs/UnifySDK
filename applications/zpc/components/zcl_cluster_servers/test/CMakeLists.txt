if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # "--allow-multiple-definition" can cause some weird symptons as we leave it
  # up to the linker which symbol gets merged into the test binary.
  add_link_options("-Wl,--unresolved-symbols=ignore-in-object-files"
                   "-Wl,--allow-multiple-definition")
endif()

# ZCL Group cluster server test
target_add_unittest(
  zcl_cluster_servers
  NAME
  zcl_group_cluster_server_test
  SOURCES
  zcl_group_cluster_server_test.c
  DEPENDS
  zpc_attribute_store
  ucl_mqtt
  uic_mqtt_mock
  zpc_dotdot_mqtt_mock
  zwave_network_management_mock
  uic_contiki_stub
  unify)

# ZCL OTA cluster server test
target_add_unittest(
  zcl_cluster_servers
  NAME
  zcl_OTA_cluster_server_test
  SOURCES
  zcl_OTA_cluster_server_test.cpp
  DEPENDS
  uic_ota_mock
  zwave_command_classes_mock
  uic_mqtt_stub)
target_include_directories(zcl_OTA_cluster_server_test PRIVATE ../src)

# ProtocolController RFTelemetry cluster server test
# We need to mock dotdot_unify_attribute_store for this test
# target_add_unittest(
#   zcl_cluster_servers
#   NAME
#   zcl_rf_telemetry_cluster_server_test
#   SOURCES
#   zcl_rf_telemetry_cluster_server_test.c
#   DEPENDS
#   zpc_attribute_store_test_helper
#   unify_dotdot_attribute_store_mock
#   zwave_controller_mock
#   zwave_network_management_mock
#   uic_dotdot_mqtt_mock
#   zwave_rx_mock
#   zwave_api_mock
#   zwave_api_transport_mock)

# Configuration Parameter Cluster Server test
target_add_unittest(
  zcl_cluster_servers
  NAME
  configuration_parameter_cluster_server_test
  SOURCES
  configuration_parameter_cluster_server_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  uic_dotdot_mqtt_mock
  zwave_network_management_mock)

# Binding Cluster Server Mapper test
target_add_unittest(
  zcl_cluster_servers
  NAME
  zcl_binding_cluster_server_test
  SOURCES
  zcl_binding_cluster_server_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  uic_dotdot_mqtt_mock
  zwave_network_management_mock)

# Scenes Cluster Mapper test
target_add_unittest(
  zcl_cluster_servers
  NAME
  zcl_scenes_cluster_server_test
  SOURCES
  zcl_scenes_cluster_server_test.c
  DEPENDS
  zpc_attribute_store_test_helper
  uic_dotdot_mqtt_mock
  zwave_network_management_mock
  zpc_dotdot_mqtt_mock
  uic_mqtt_stub
  uic_contiki_stub
  unify)

target_include_directories(zcl_scenes_cluster_server_test PRIVATE ../src)
