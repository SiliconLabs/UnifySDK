find_package(Boost REQUIRED)

# ZCL Group cluster server
add_library(zcl_group_cluster_server src/zcl_group_cluster_server.cpp)
target_include_directories(
  zcl_group_cluster_server
  PUBLIC include ${Boost_INCLUDE_DIRS}
  PRIVATE src)
target_link_libraries(
  zcl_group_cluster_server
  PUBLIC zcl_cluster_servers
  PRIVATE uic_attribute_store zpc_attribute_store zpc_utils
          dotdot_mqtt_topics_handler dotdot_attributes uic_mqtt ucl_node_state)
target_interface_libraries(zcl_group_cluster_server zwave_controller)

# ZCL OTA Cluster Server
add_library(zcl_OTA_cluster_server src/zcl_OTA_cluster_server.cpp)
target_link_libraries(zcl_OTA_cluster_server
                      PRIVATE
                        zpc_attribute_store
                        uic_ota
                        zwave_command_classes
                        uic_attribute_resolver)

# ZCL Cluster server library
add_library(zcl_cluster_servers src/zcl_cluster_servers.cpp
                                src/zcl_cluster_servers_helpers.cpp)
target_include_directories(
  zcl_cluster_servers
  PUBLIC include
  PRIVATE src)

target_link_libraries(
  zcl_cluster_servers PUBLIC zcl_OTA_cluster_server zcl_group_cluster_server
                             zwave_handlers)

# Unit tests
add_subdirectory(test)

# Mocks
target_add_mock(zcl_cluster_servers)
