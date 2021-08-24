add_library(
  zigpc_diagnostics
  src/zigpc_diagnostics_manager.cpp src/zigpc_diagnostics_mqtt_helper.cpp)

  target_include_directories(
  zigpc_diagnostics
  PUBLIC include
  PRIVATE src src/health_metrics)

  add_subdirectory(test)

  target_link_libraries(
    zigpc_diagnostics  PRIVATE zigpc_net_mgmt zigpc_gateway zigpc_common  uic_contiki uic_log  uic_mqtt)
