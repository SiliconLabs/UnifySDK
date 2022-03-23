# Library
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)
set(NETWORK_STATE_SOURCES src/network_monitor.cpp
                          src/network_monitor_span_persistence.c
                          src/smart_start_list_monitor.cpp
                          src/failing_node_monitor.cpp
                          src/network_monitor_fixt.c)

message(STATUS "Boost includes: " ${Boost_INCLUDE_DIRS})

add_library(network_monitor ${NETWORK_STATE_SOURCES})
target_include_directories(network_monitor PUBLIC include ${Boost_INCLUDE_DIRS}
                                           PRIVATE src)
target_link_libraries(network_monitor
  PUBLIC
    zwave_command_classes
    zwave_definitions
    zpc_attribute_store
    uic_attribute_resolver
    uic_mqtt
    zpc_utils
    ucl_mqtt_node_interview
    ucl_node_state
    zcl_cluster_servers
    ucl_definitions
    zwave_controller
    uic_smartstart_management
    zwave_volatile_attr_poll_manager_target
    uic_attribute_utils
)

add_subdirectory(test)
add_mock(failing_node_monitor_mock src/failing_node_monitor.h)
target_interface_libraries(failing_node_monitor_mock zwave_definitions)
