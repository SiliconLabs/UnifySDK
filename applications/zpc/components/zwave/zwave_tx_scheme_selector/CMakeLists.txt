add_library(zwave_tx_scheme_selector src/zwave_tx_scheme_selector.c)

target_include_directories(zwave_tx_scheme_selector PUBLIC include)

target_link_libraries(zwave_tx_scheme_selector PRIVATE zwave_controller
                                            zwave_api zwave_tx uic_log)
install(TARGETS zwave_tx_scheme_selector LIBRARY DESTINATION lib)

target_interface_libraries(zwave_tx_scheme_selector zwave_tx_groups)

# Mocks
target_add_mock(zwave_tx_scheme_selector)

add_subdirectory(test)
