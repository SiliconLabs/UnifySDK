find_program(ZAP_EXECUTABLE "zap" REQUIRED)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)
run_zap(zap/gen-templates.json)

# In order to make ZAP run, there needs to be a dependency to each files
# specified in zap/gen-templates.json
add_library(
  zigpc_zcl_util
  src/zcl_util.cpp ${CMAKE_CURRENT_BINARY_DIR}/src/zcl_maps.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/include/zcl_definitions.h)

target_include_directories(
  zigpc_zcl_util
  PRIVATE src ${CMAKE_CURRENT_BINARY_DIR}/src
  PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/include)

target_link_libraries(
  zigpc_zcl_util
  INTERFACE zigpc_common
  PRIVATE zigpc_common uic_log)

add_mock(zigpc_zcl_util_mock include/zcl_util.h
         ${CMAKE_CURRENT_BINARY_DIR}/include/zcl_definitions.h)
target_interface_libraries(zigpc_zcl_util_mock zigpc_common_mock)

add_subdirectory(test)
