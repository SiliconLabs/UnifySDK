add_library(zigpc_controller src/zigpc_controller.cpp
                             src/zigpc_controller_events.cpp)

target_include_directories(
  zigpc_controller
  PUBLIC include
  PRIVATE src)

target_link_libraries(
  zigpc_controller
  PRIVATE zigpc_attribute_management
          zigpc_zcl_util
          zigpc_datastore
          zigpc_ota_zigbee
          zigpc_gateway
          zigpc_common
          zigpc_net_mgmt
          zigpc_ucl
          uic_log
          uic_dotdot_mqtt
          uic_mqtt
          uic_contiki)

target_add_mock(zigpc_controller)

add_subdirectory(test)
