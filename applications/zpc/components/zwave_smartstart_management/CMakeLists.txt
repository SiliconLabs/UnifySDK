find_package(Boost REQUIRED)

add_library(
  zwave_smartstart_management
  src/zwave_smartstart_management.cpp)

target_include_directories(
  zwave_smartstart_management
  PUBLIC
    include
  PRIVATE
    ../zwave/zwave_transports/s2/libs/zw-libs2/include
    ${Boost_INCLUDE_DIRS}
)


target_link_libraries(zwave_smartstart_management
                      PUBLIC zwave_controller
                             zwave_network_management
                             zpc_attribute_store
                      PRIVATE
                             uic_mqtt uic_smartstart_management zpc_config)

add_subdirectory(test)
target_add_mock(zwave_smartstart_management)
