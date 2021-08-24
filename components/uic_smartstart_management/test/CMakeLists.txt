target_add_unittest(uic_smartstart_management
SOURCES smartstart_management_test.cpp
DEPENDS
    uic_mqtt_mock
    uic_log
    uic_contiki_stub)