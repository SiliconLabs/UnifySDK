run_zap(zap/gen-templates.json)

# Find nlohmann_json, unfortunately the find_package doesn't work for this when
# cross compiling, thus we use the simplified method find_path instead
find_path(nlohmann_json_include nlohmann/json.hpp REQUIRED)

# In order to make ZAP run, there needs to be a dependency to each files
# specified in zap/gen-templates.json.
add_library(
  zigpc_command_mapper
  src/zigpc_command_mapper.c
  ${ZAP_CURRENT_OUTPUT_DIR}/src/zigpc_command_mapper_handlers.cpp
  ${ZAP_CURRENT_OUTPUT_DIR}/src/zigpc_command_mapper_bygroup_handlers.cpp
  ${ZAP_CURRENT_OUTPUT_DIR}/src/zigpc_cmdmapper_gen_cmd_publish.cpp)

target_include_directories(
  zigpc_command_mapper
  PUBLIC include
  PRIVATE src ${nlohmann_json_include})

target_link_libraries(
  zigpc_command_mapper
  PRIVATE zigpc_zcl_util
          zigpc_attribute_management
          zigpc_gateway
          zigpc_common
          zigpc_datastore
          zigpc_group_mgmt
          zigpc_ucl
          zigpc_zcl_command_parser
          unify)

if(BUILD_TESTING)
  add_subdirectory(test)

  target_add_mock(zigpc_command_mapper)
endif()
