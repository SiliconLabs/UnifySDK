find_path(nlohmann_json_include nlohmann/json.hpp REQUIRED)

add_library(unify_node_state_monitor SHARED src/unify_node_state_monitor.cpp)

target_include_directories(
  unify_node_state_monitor
  PUBLIC include
  PRIVATE ${nlohmann_json_include})

target_link_libraries(
  unify_node_state_monitor PUBLIC uic_contiki uic_contiki_platform uic_mqtt
                                  uic_dotdot_mqtt uic_log)
add_subdirectory(test)
