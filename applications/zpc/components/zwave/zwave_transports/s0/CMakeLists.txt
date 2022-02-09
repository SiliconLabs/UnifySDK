# Library
add_library(zwave_s0 src/zwave_s0_network.c src/zwave_s0_sm.c
                     src/zwave_s0_transport.c)
# Make libs2 build as if its built by ZIPGW and ZW_CONTROLLER
target_compile_definitions(zwave_s0 PRIVATE ZIPGW ZW_CONTROLLER)

target_include_directories(zwave_s0 PUBLIC include)
target_link_libraries(
  zwave_s0
  PUBLIC uic_contiki uic_contiki_platform zwave_network_management
  PRIVATE s2crypto aes zwave_controller
  # PRIVATE s2_controller # for keystor and aes finctions
)
install(TARGETS zwave_s0 LIBRARY DESTINATION lib)

# Unit tests
add_subdirectory(test)

# Mocks
target_add_mock(zwave_s0)
add_mock(zwave_s0_transport_mock include/zwave_s0_transport.h)
target_interface_libraries(zwave_s0_transport_mock zwave_controller)
