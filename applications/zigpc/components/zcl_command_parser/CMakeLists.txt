find_program(ZAP_EXECUTABLE "zap" REQUIRED)
run_zap(zap/gen-templates.json)

# In order to make ZAP run, there needs to be a dependency to each files
# specified in zap/gen-templates.json.
add_library(
  zigpc_zcl_command_parser
  src/zcl_command_parser_callbacks.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/src/zcl_command_parser.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/src/zcl_command_parser_int.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/zcl_command_parser.h)

target_include_directories(
  zigpc_zcl_command_parser
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src
  PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include)

target_link_libraries(
  zigpc_zcl_command_parser
  INTERFACE zigpc_common zigpc_zcl_util
  PRIVATE zigpc_gateway uic_multi_invoke uic_log)

add_mock(zigpc_zcl_command_parser_mock
         ${CMAKE_CURRENT_BINARY_DIR}/include/zcl_command_parser.h)
target_interface_libraries(zigpc_zcl_command_parser_mock zigpc_common_mock
                           zigpc_zcl_util_mock)

add_subdirectory(test)
