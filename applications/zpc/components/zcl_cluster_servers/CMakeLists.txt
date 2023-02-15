find_package(Boost REQUIRED)

# Find nlohmann_json, unfortunately the find_package doesn't work for this when
# cross compiling, thus we use the simplified method find_path instead
find_path(nlohmann_json_include nlohmann/json.hpp REQUIRED)

# ZCL Cluster server library
add_library(
  zcl_cluster_servers
  src/configuration_parameter_cluster_server.cpp
  src/zcl_binding_cluster_server.cpp
  src/zcl_cluster_servers.cpp
  src/zcl_cluster_servers_helpers.cpp
  src/zcl_group_cluster_server.cpp
  src/zcl_OTA_cluster_server.cpp
  src/zcl_rf_telemetry_cluster_server.c
  src/zcl_scenes_cluster_server.cpp)

target_include_directories(
  zcl_cluster_servers
  PUBLIC include
  PRIVATE src ${nlohmann_json_include} ${Boost_INCLUDE_DIRS})

target_link_libraries(
  zcl_cluster_servers
  PUBLIC zwave_handlers zwave_definitions
  PRIVATE unify
          zpc_attribute_store
          zpc_utils
          zpc_dotdot_mqtt
          ucl_mqtt
          zwave_command_classes
          zwave_api_transport
          dotdot_mapper
          zwave_network_management)

if(BUILD_TESTING)
  # Unit tests
  add_subdirectory(test)

  # Mocks
  target_add_mock(zcl_cluster_servers)
endif()
