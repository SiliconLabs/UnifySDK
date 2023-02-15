# Find nlohmann_json, unfortunately the find_package doesn't work for this when
# cross compiling, thus we use the simplified method find_path instead
find_path(nlohmann_json_include nlohmann/json.hpp REQUIRED)

add_library(
  zigpc_diagnostics
  src/zigpc_diagnostics_manager.cpp src/zigpc_diagnostics_mqtt_helper.cpp)

  target_include_directories(
  zigpc_diagnostics
  PUBLIC include
  PRIVATE src src/health_metrics ${nlohmann_json_include})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

target_link_libraries(
    zigpc_diagnostics  PRIVATE zigpc_net_mgmt zigpc_gateway zigpc_common  unify)
