add_library(transport_service src/transport_service.c)
target_include_directories(
  transport_service
  PUBLIC include
  PRIVATE src)

target_link_libraries(
  transport_service
  PUBLIC zwave_definitions
  PRIVATE zwave_controller zwave_tx)
target_compile_options(transport_service PRIVATE -Wno-unused-variable)

if(BUILD_TESTING)
  add_subdirectory(test)
  target_add_mock(transport_service) # target_add_mock() is mocked in Z-Wave
                                     # repo!
endif()
