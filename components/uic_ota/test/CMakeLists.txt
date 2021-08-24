target_add_unittest(
  uic_ota
  NAME
  uic_ota_test
  SOURCES
  uic_ota_test.cpp
  DEPENDS
  uic_mqtt_stub
  uic_contiki_stub
  EXCLUDE
  uic_mqtt)

target_include_directories(uic_ota_test PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
