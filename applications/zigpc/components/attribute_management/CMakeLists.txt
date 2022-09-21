run_zap(zap/gen-templates.json)

set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)

add_library(
  zigpc_attribute_management
  ${ZAP_CURRENT_OUTPUT_DIR}/src/zigpc_attrmgmt_publish.cpp
  src/attribute_management_process.c
  src/attribute_management.c
  src/attribute_map.cpp
  src/zigpc_attrmgmt_gateway.c
  src/zigpc_attrmgmt_receive.c
  src/zigpc_attrmgmt_reporting.cpp
  src/zigpc_attrmgmt_send.cpp
  src/zigpc_attrmgmt_poll.cpp)

target_include_directories(
  zigpc_attribute_management
  PUBLIC include $ {Boost_INCLUDE_DIRS}
  PRIVATE src)

target_link_libraries(
  zigpc_attribute_management
  PRIVATE zigpc_zcl_profiles
          zigpc_gateway
          zigpc_common
          zigpc_config
          zigpc_ucl
          uic_contiki
          uic_log
          uic_dotdot_mqtt)

add_subdirectory(test)

target_add_mock(zigpc_attribute_management)
