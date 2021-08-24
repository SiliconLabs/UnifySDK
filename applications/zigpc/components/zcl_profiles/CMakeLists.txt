find_program(ZAP_EXECUTABLE "zap" REQUIRED)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)
run_zap(zap/gen-templates.json)

# In order to make ZAP run, there needs to be a dependency to each files
# specified in zap/gen-templates.json: zcl_profiles_mapper.h/c
add_library(
  zigpc_zcl_profiles
  platform/${COMPATIBLE_PLATFORM}/zcl_profiles_command_fields.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/src/zcl_attribute_lists.cpp
  src/zcl_attribute_info.cpp)

target_include_directories(zigpc_zcl_profiles PUBLIC include)
target_include_directories(zigpc_zcl_profiles PRIVATE src ${Boost_INCLUDE_DIRS}
                                                platform/${COMPATIBLE_PLATFORM}
                                                ${CMAKE_CURRENT_BINARY_DIR}/src)

target_link_libraries(zigpc_zcl_profiles PRIVATE zigpc_common zigpc_zcl_util uic_log)

# create mocks

add_mock( zigpc_zcl_profiles_mock
    include/zcl_attribute_info.h
    include/zcl_profiles.h)

target_interface_libraries(zigpc_zcl_profiles_mock zigpc_common_mock zigpc_zcl_util_mock)
