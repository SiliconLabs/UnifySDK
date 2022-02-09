add_library(
  zigpc_discovery
  src/zigpc_discovery.cpp src/zigpc_discovery_callbacks.cpp
  src/zigpc_discovery_events.cpp src/zigpc_discovery_process.cpp
  src/zigpc_discovery_timers.cpp)

target_include_directories(
  zigpc_discovery
  PRIVATE src
  PUBLIC include)

target_link_libraries(
  zigpc_discovery
  INTERFACE zigpc_common
  PRIVATE zigpc_datastore zigpc_gateway uic_contiki uic_log)

target_add_mock(zigpc_discovery)

add_subdirectory(test)
