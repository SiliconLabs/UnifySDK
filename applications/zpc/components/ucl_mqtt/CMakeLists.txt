set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)
# Find nlohmann_json, unfortunately the find_package doesn't work for this when
# cross compiling, thus we use the simplified method find_path instead
find_path(nlohmann_json_include nlohmann/json.hpp REQUIRED)

# UCL MQTT library
add_library(
  ucl_mqtt
  src/ucl_mqtt.c src/ucl_mqtt_node_interview.c src/ucl_network_management.cpp
  src/ucl_nm_neighbor_discovery.cpp src/ucl_node_state.cpp)

target_link_libraries(
  ucl_mqtt
  PUBLIC ucl_definitions zwave_definitions
  PRIVATE unify zpc_attribute_store)

target_include_directories(
  ucl_mqtt
  PUBLIC include ${Boost_INCLUDE_DIRS}
  PRIVATE ${nlohmann_json_include})

if(BUILD_TESTING)
  # Unit tests
  add_subdirectory(test)

  # Mocks
  add_mock(ucl_mqtt_mock include/ucl_mqtt.h)
  target_interface_libraries(ucl_mqtt_mock zpc_utils)

  add_mock(ucl_mqtt_node_interview_mock include/ucl_mqtt_node_interview.h)
  target_interface_libraries(ucl_mqtt_node_interview_mock zpc_utils)

  add_mock(ucl_nm_neighbor_discovery_mock src/ucl_nm_neighbor_discovery.h)
  target_interface_libraries(
    ucl_nm_neighbor_discovery_mock zwave_controller_mock zpc_utils
    zpc_attribute_store_mock)

  # add_mock(ucl_network_management_mock src/ucl_network_management.h)
  # target_link_libraries( ucl_network_management_mock PUBLIC zwave_definitions
  # PRIVATE unify)

endif()
