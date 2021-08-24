add_library(
  zigpc_node_mgmt
  src/zigpc_node_map.cpp src/zigpc_node_mgmt.c src/zigpc_node_mgmt_gateway.c
  src/zigpc_node_mgmt_process.c src/zigpc_node_mgmt.cpp)

target_include_directories(
  zigpc_node_mgmt
  PUBLIC include
  PRIVATE src)

target_link_libraries(
  zigpc_node_mgmt PRIVATE zigpc_attribute_management zigpc_ota_zigbee zigpc_gateway zigpc_common zigpc_net_mgmt
                          uic_contiki uic_log uic_dotdot_mqtt)

target_add_mock(zigpc_node_mgmt)
