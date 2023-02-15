run_zap(zap/gen-templates.json)

# In order to make ZAP run, there needs to be a dependency to each files
# specified in zap/gen-templates.json.
add_library(
  zigpc_zcl_command_parser
  src/zcl_command_parser_callbacks.cpp
  ${ZAP_CURRENT_OUTPUT_DIR}/src/zcl_command_parser.cpp
  ${ZAP_CURRENT_OUTPUT_DIR}/src/zcl_command_parser_int.h
  ${ZAP_CURRENT_OUTPUT_DIR}/include/zcl_command_parser.h)

target_include_directories(
  zigpc_zcl_command_parser
  PRIVATE ${ZAP_CURRENT_OUTPUT_DIR}/src
  PUBLIC ${ZAP_CURRENT_OUTPUT_DIR}/include)

target_link_libraries(
  zigpc_zcl_command_parser
  INTERFACE zigpc_common zigpc_zcl_util
  PRIVATE zigpc_gateway unify)

if(BUILD_TESTING)
  add_mock(zigpc_zcl_command_parser_mock
          ${ZAP_CURRENT_OUTPUT_DIR}/include/zcl_command_parser.h)
  target_interface_libraries(zigpc_zcl_command_parser_mock zigpc_common_mock
                            zigpc_zcl_util_mock)

  add_subdirectory(test)
endif()
