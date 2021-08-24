# Library
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)
set(NETWORK_STATE_SOURCES src/network_monitor.cpp
                          src/network_monitor_fixt.c)

message(STATUS "Boost includes: " ${Boost_INCLUDE_DIRS})

add_library(network_monitor ${NETWORK_STATE_SOURCES})
target_include_directories(network_monitor PUBLIC include ${Boost_INCLUDE_DIRS}
                                           PRIVATE src)
target_link_libraries(network_monitor
  PUBLIC
    zwave_controller
    zpc_attribute_store
    uic_attribute_resolver
    uic_mqtt
    zpc_utils
    ucl_mqtt_node_interview
    ucl_definitions
    zwave_command_classes
)

add_subdirectory(test)