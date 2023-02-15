# Library
add_library(transport_service_wrapper src/zwave_transport_service_wrapper.c)

target_include_directories(
  transport_service_wrapper
  PUBLIC include
  PRIVATE src transport_service)

target_link_libraries(
  transport_service_wrapper
  PUBLIC transport_service
  PRIVATE zwave_controller zwave_tx)

add_subdirectory(transport_service)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
