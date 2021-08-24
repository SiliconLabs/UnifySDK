add_library(
  zigpc_gateway
  src/zigpc_gateway.c src/zigpc_gateway_datastore.cpp
  src/zigpc_gateway_dispatch.c src/zigpc_gateway_notify.c
  src/zigpc_gateway_process.c src/gateway_sleepy_queue.cpp
  src/zigpc_gateway_counters.cpp)

target_include_directories(
  zigpc_gateway
  PUBLIC include
  PRIVATE src)

target_link_libraries(
  zigpc_gateway
  INTERFACE zigpc_zcl_util
  PRIVATE z3gateway
          zigpc_common
          zigpc_config
          zigpc_zcl_util
          zigpc_node_mgmt
          uic_dotdot_mqtt
          uic_contiki
          uic_datastore
          uic_log)

# create mocks
add_mock(zigpc_gateway_mock include/zigpc_gateway.h
         include/zigpc_gateway_notify.h)
target_interface_libraries(zigpc_gateway_mock zigpc_common_mock
                           zigpc_zcl_util_mock)

add_subdirectory(test)
