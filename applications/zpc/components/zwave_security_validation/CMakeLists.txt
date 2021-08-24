add_library(zwave_security_validation
  src/zwave_security_validation.c
)

target_include_directories(
  zwave_security_validation
  PUBLIC include
  PRIVATE src)
target_link_libraries(zwave_security_validation
  uic_log
  uic_attribute_store
  zpc_attribute_store
  zwave_controller
)

# Mocks
target_add_mock(zwave_security_validation)

add_subdirectory(test)
