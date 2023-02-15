set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)
run_zap(zap/gen-templates.json)

# In order to make ZAP run, there needs to be a dependency to each files
# specified in zap/gen-templates.json
add_library(
  zigpc_zcl_util src/zcl_util.cpp ${ZAP_CURRENT_OUTPUT_DIR}/src/zcl_maps.cpp
                 ${ZAP_CURRENT_OUTPUT_DIR}/include/zcl_definitions.h)

target_include_directories(
  zigpc_zcl_util
  PRIVATE src ${ZAP_CURRENT_OUTPUT_DIR}/src
  PUBLIC include ${ZAP_CURRENT_OUTPUT_DIR}/include)

target_link_libraries(
  zigpc_zcl_util
  INTERFACE zigpc_common
  PRIVATE zigpc_common unify)

if(BUILD_TESTING)
  add_mock(zigpc_zcl_util_mock include/zcl_util.h
          ${ZAP_CURRENT_OUTPUT_DIR}/include/zcl_definitions.h)
  target_interface_libraries(zigpc_zcl_util_mock zigpc_common_mock)

  add_subdirectory(test)
endif()
