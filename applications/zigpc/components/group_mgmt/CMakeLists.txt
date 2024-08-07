add_library(zigpc_group_mgmt STATIC src/zigpc_group_map.cpp src/zigpc_group_mgmt.c
                                    src/zigpc_group_mgmt_mqtt.cpp)

set_property(TARGET zigpc_group_mgmt PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(
  zigpc_group_mgmt
  PUBLIC include
  PRIVATE src)

target_link_libraries(
  zigpc_group_mgmt
  PRIVATE zigpc_common zigpc_datastore zigpc_zcl_util zigpc_zcl_command_parser
  unify)

if(BUILD_TESTING)
  add_subdirectory(test)

  add_mock(zigpc_group_mgmt_mock include/zigpc_group_mgmt.h)
  target_interface_libraries(zigpc_group_mgmt_mock zigpc_common zigpc_zcl_util)
endif()
