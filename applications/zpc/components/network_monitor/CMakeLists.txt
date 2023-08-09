# Library
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)
set(NETWORK_STATE_SOURCES
    src/network_monitor.cpp
    src/network_monitor_span_persistence.c
    src/smart_start_list_monitor.cpp
    src/keep_sleeping_nodes_alive.cpp
    src/failing_node_monitor.cpp
    src/network_monitor_fixt.c
    src/network_monitor_utils.cpp)

message(STATUS "Boost includes: " ${Boost_INCLUDE_DIRS})

add_library(network_monitor ${NETWORK_STATE_SOURCES})
target_include_directories(
  network_monitor
  PUBLIC include ${Boost_INCLUDE_DIRS}
  PRIVATE src)
target_link_libraries(
  network_monitor
  INTERFACE zwave_definitions
  PUBLIC zpc_attribute_store unify ucl_mqtt zcl_cluster_servers
  PRIVATE zwave_command_classes zwave_s2)

if(BUILD_TESTING)
  add_subdirectory(test)
  add_mock(failing_node_monitor_mock src/failing_node_monitor.h)

  target_interface_libraries(failing_node_monitor_mock zwave_definitions unify)
endif()
