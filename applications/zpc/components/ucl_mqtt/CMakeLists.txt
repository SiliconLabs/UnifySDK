set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)

# UCL node state
add_library(ucl_node_state src/ucl_node_state.cpp)
target_include_directories(ucl_node_state PUBLIC include $ {
                                                 Boost_INCLUDE_DIRS})
target_link_libraries(
  ucl_node_state
  uic_contiki
  uic_attribute_store
  zpc_attribute_store
  zpc_utils
  uic_mqtt
  ucl_definitions
  ucl_mqtt_node_interview
  ucl_network_management)
target_interface_libraries(ucl_node_state zwave_controller)

# UCL network management
add_library(ucl_network_management src/ucl_network_management.cpp)
target_include_directories(ucl_network_management PUBLIC include $ {
                                                         Boost_INCLUDE_DIRS})
target_link_libraries(ucl_network_management zwave_controller zpc_utils uic_mqtt ucl_nm_neighbor_discovery)

# UCL network management neighbor discovery
add_library(ucl_nm_neighbor_discovery src/ucl_nm_neighbor_discovery.cpp)
target_include_directories(ucl_nm_neighbor_discovery PRIVATE src)
target_link_libraries(ucl_nm_neighbor_discovery zwave_controller zpc_utils zwave_network_management)

# UCL MQTT Node interview
add_library(ucl_mqtt_node_interview src/ucl_mqtt_node_interview.c)
target_include_directories(ucl_mqtt_node_interview
  PUBLIC
  include)
target_link_libraries(ucl_mqtt_node_interview
  zpc_attribute_store
  zwave_controller
  zpc_utils
)

# The main UCL MQTT library
add_library(ucl_mqtt src/ucl_mqtt.c)
target_link_libraries(ucl_mqtt
  PUBLIC
    ucl_mqtt_node_interview
    ucl_node_state
    ucl_network_management
)

# Unit tests
add_subdirectory(test)

# Mocks
add_mock(ucl_mqtt_mock include/ucl_mqtt.h)
target_interface_libraries(ucl_mqtt_mock zpc_utils zwave_controller)

add_mock(ucl_mqtt_node_interview_mock include/ucl_mqtt_node_interview.h)
target_interface_libraries(ucl_mqtt_node_interview_mock zpc_utils zwave_controller)

add_mock(ucl_nm_neighbor_discovery_mock src/ucl_nm_neighbor_discovery.h)
target_interface_libraries(ucl_nm_neighbor_discovery_mock zwave_controller zpc_utils zwave_network_management)