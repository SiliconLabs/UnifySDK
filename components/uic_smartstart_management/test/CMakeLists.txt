add_library(uic_smartstart_management_shared SHARED
  $<TARGET_OBJECTS:uic_smartstart_management>) 
target_link_libraries(uic_smartstart_management_shared unify)

target_add_unittest(uic_smartstart_management_shared
NAME uic_smartstart_management_test
SOURCES smartstart_management_test.cpp
DEPENDS
    uic_mqtt_mock
    uic_contiki_stub
    )
