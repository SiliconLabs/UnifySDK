set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)
run_zap(zap/gen-templates.json)

# In order to make ZAP run, there needs to be a dependency to each files
# specified in zap/gen-templates.json: zcl_profiles_mapper.h/c
add_library(
  zigpc_zcl_profiles ${ZAP_CURRENT_OUTPUT_DIR}/src/zcl_attribute_lists.cpp
                     src/zcl_attribute_info.cpp)

target_include_directories(zigpc_zcl_profiles PUBLIC include)
target_include_directories(
  zigpc_zcl_profiles
  PRIVATE src ${Boost_INCLUDE_DIRS} platform/${COMPATIBLE_PLATFORM}
          ${ZAP_CURRENT_OUTPUT_DIR}/src)

target_link_libraries(zigpc_zcl_profiles PRIVATE zigpc_common zigpc_zcl_util
                                                 uic_log)

# create mocks

add_mock(zigpc_zcl_profiles_mock include/zcl_attribute_info.h)

target_interface_libraries(zigpc_zcl_profiles_mock zigpc_common_mock
                           zigpc_zcl_util_mock)
