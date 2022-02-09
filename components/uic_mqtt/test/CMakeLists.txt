target_add_unittest(uic_mqtt_client
SOURCES mqtt_client_test.cpp
DEPENDS
    uic_mqtt_wrapper_mock cmock)
target_include_directories(uic_mqtt_client PRIVATE ../src)

target_add_unittest(uic_mqtt
SOURCES uic_mqtt_test.c
DEPENDS
    uic_contiki_stub
    uic_config_mock
    uic_main_mock
    uic_main_fd_mock
    uic_mqtt_client_mock
    uic_mqtt_wrapper_mock)
target_include_directories(uic_mqtt_test PRIVATE ../src)
