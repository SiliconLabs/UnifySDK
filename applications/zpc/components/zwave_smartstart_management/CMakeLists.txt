find_package(Boost REQUIRED)

add_library(zwave_smartstart_management src/zwave_smartstart_management.cpp)

target_include_directories(
  zwave_smartstart_management
  PUBLIC include
  PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(
  zwave_smartstart_management
  PUBLIC zwave_controller zwave_network_management zpc_attribute_store
  PRIVATE s2_controller unify zpc_config)

if(BUILD_TESTING)
  add_subdirectory(test)
  target_add_mock(zwave_smartstart_management)
endif()
