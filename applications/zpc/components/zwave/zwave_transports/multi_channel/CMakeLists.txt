# Library
add_library(zwave_multi_channel
  src/zwave_multi_channel_transport.c
)

target_include_directories(zwave_multi_channel
  PUBLIC include
)

target_link_libraries(zwave_multi_channel
  PUBLIC zwave_controller
)

target_add_mock(zwave_multi_channel)
add_subdirectory(test)
