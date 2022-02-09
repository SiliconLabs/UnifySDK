find_program(ZAP_EXECUTABLE "zap" REQUIRED)
run_zap(zap/gen-templates.json)

# Find nlohmann_json, unfortunately the find_package doesn't work for this when
# cross compiling, thus we use the simplified method find_path instead
find_path(nlohmann_json_include nlohmann/json.hpp REQUIRED)

# In order to make ZAP run, there needs to be a dependency to each files
# specified in zap/gen-templates.json.
add_library(
  zigpc_command_mapper
  src/zigpc_command_mapper.c
  ${CMAKE_CURRENT_BINARY_DIR}/src/zigpc_command_mapper_handlers.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/src/zigpc_command_mapper_bygroup_handlers.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/src/zigpc_cmdmapper_gen_cmd_publish.cpp)

target_include_directories(
  zigpc_command_mapper
  PUBLIC include
  PRIVATE src)

target_link_libraries(
  zigpc_command_mapper
  PRIVATE zigpc_zcl_util
          zigpc_gateway
          zigpc_common
          zigpc_datastore
          zigpc_group_mgmt
          zigpc_ucl
          zigpc_zcl_command_parser
          uic_dotdot_mqtt
          uic_log)

add_subdirectory(test)

target_add_mock(zigpc_command_mapper)
