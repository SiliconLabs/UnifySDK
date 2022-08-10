target_add_unittest(
  unify_node_state_monitor
  SOURCES
  unify_node_state_monitor_test.cpp
  DEPENDS
  uic_contiki_stub
  uic_mqtt_stub
  EXCLUDE
  uic_mqtt)

add_executable(unify_node_state_monitor_demo unify_node_state_monitor_demo.cpp)
target_link_libraries(unify_node_state_monitor_demo unify_node_state_monitor
                      uic_main)
