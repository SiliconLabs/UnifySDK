# Library
add_library(zwave_rx src/zwave_rx.c src/zwave_rx_fixt.c src/zwave_rx_process.c
                     src/zwave_rx_zwapi_callbacks.c)

target_include_directories(zwave_rx PUBLIC include)

target_link_libraries(zwave_rx PUBLIC zpc_config PRIVATE zwave_controller uic_main_fd)

# CMock for Z - Wave RX
target_add_mock(zwave_rx)

add_subdirectory(test)
