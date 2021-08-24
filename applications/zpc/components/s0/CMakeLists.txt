# Library
add_library(zwave_s0
  src/zwave_s0_network.c
  src/zwave_s0_sm.c
)

target_include_directories(zwave_s0
  PUBLIC include
  PRIVATE src
          ${ZPC_SOURCE_DIR}/components/s2/libs/zw-libs2/include
)
target_link_libraries(zwave_s0
  PUBLIC zwave_controller
         zwave_tx
         zwave_rx
         zwave_transports
)
add_subdirectory(test)
target_add_mock(zwave_s0)
