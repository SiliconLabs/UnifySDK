add_library(zwave_security_validation src/zwave_security_validation.c)

target_include_directories(
  zwave_security_validation
  PUBLIC include
  PRIVATE src)
target_link_libraries(zwave_security_validation uic_log zwave_controller
                      zwave_tx_scheme_selector)
install(TARGETS zwave_security_validation LIBRARY DESTINATION lib)

# Mocks
target_add_mock(zwave_security_validation)

add_subdirectory(test)
