set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)
# Find nlohmann_json, unfortunately the find_package doesn't work for this when
# cross compiling, thus we use the simplified method find_path instead
find_path(nlohmann_json_INCLUDE_DIRS nlohmann/json.hpp REQUIRED)

add_library(zigpc_ucl src/zigpc_ucl.cpp src/zigpc_ucl_mqtt.cpp
                      src/zigpc_ucl_node_state.cpp src/zigpc_ucl_pc_nwmgmt.cpp)

target_include_directories(
  zigpc_ucl
  PRIVATE src ${Boost_INCLUDE_DIRS} ${nlohmann_json_INCLUDE_DIRS}
  PUBLIC include)

target_link_libraries(zigpc_ucl PRIVATE zigpc_common zigpc_datastore
                                        zigpc_net_mgmt unify)

if(BUILD_TESTING)
  target_add_mock(zigpc_ucl)

  add_subdirectory(test)
endif()
