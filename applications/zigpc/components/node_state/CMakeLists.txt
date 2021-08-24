set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)

add_library(
  zigpc_node_state
  platform/${COMPATIBLE_PLATFORM}/zigpc_node_state_topic_update.cpp
  src/zigpc_node_state.c src/zigpc_node_state_observers.c)

target_include_directories(
  zigpc_node_state
  PUBLIC include
  PRIVATE src)

target_link_libraries(zigpc_node_state PRIVATE zigpc_net_mgmt zigpc_common
                                               uic_mqtt uic_log)

# add_subdirectory(test)

target_add_mock(zigpc_node_state)
