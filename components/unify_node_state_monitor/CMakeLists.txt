find_path(nlohmann_json_include nlohmann/json.hpp REQUIRED)

add_library(unify_node_state_monitor OBJECT src/unify_node_state_monitor.cpp)

target_include_directories(
  unify_node_state_monitor
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  PRIVATE ${nlohmann_json_include})
target_link_libraries(
  unify_node_state_monitor PUBLIC uic_contiki uic_contiki_platform uic_mqtt
                                  uic_dotdot_mqtt uic_log)

install_headers(unify_node_state_monitor uic libuic-dev)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
