# Source/Include
add_library(zigpc_ota_zigbee src/zigpc_ota_zigbee.cpp
                             src/zigpc_ota_zigbee_gateway.cpp)

# Lib
target_include_directories(
  zigpc_ota_zigbee
  PUBLIC include
  PRIVATE src)

target_link_libraries(
  zigpc_ota_zigbee
  PRIVATE zigpc_common
          zigpc_datastore
          zigpc_gateway
          zigpc_zcl_util
          unify
          zigpc_config)

if(BUILD_TESTING)
  add_subdirectory(test)

  target_add_mock(zigpc_ota_zigbee)
endif()
