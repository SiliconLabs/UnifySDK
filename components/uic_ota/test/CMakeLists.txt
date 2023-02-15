add_library(uic_ota_shared SHARED
            $<TARGET_OBJECTS:uic_ota>)
target_link_libraries(uic_ota_shared unify)
target_include_directories(
  uic_ota_shared
  PUBLIC $<TARGET_PROPERTY:uic_ota,INCLUDE_DIRECTORIES>)

target_add_unittest(
  uic_ota_shared
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
