set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)

# Mocks
add_mock(zigpc_protctrl_nwmgmt_write_mock
         ../platform/${COMPATIBLE_PLATFORM}/zigpc_protctrl_nwmgmt_write.h)
target_interface_libraries(zigpc_protctrl_nwmgmt_write_mock zigpc_common_mock)

add_mock(
  zigpc_protctrl_nwmgmt_write_callback_mock
  ../platform/${COMPATIBLE_PLATFORM}/zigpc_protctrl_nwmgmt_write_callback.h)
target_interface_libraries(zigpc_protctrl_nwmgmt_write_callback_mock
                           zigpc_common_mock)

add_mock(zigpc_protctrl_process_send_mock ../src/zigpc_protctrl_process_send.h)

# Listener Tests
target_add_unittest(
  zigpc_protctrl
  NAME
  zigpc_protctrl_observers_test
  SOURCES
  zigpc_protctrl_observers_test.c
  DEPENDS
  zigpc_protctrl_process_send_mock
  zigpc_common_mock
  zigpc_net_mgmt_mock)

# NWMGMT Tests
target_add_unittest(
  zigpc_protctrl
  NAME
  zigpc_protctrl_nwmgmt_test
  SOURCES
  zigpc_protctrl_nwmgmt_test.c
  EXCLUDE
  zigpc_protctrl_process.c
  zigpc_protctrl_nwmgmt_write_callback.c
  zigpc_protctrl_observers.c
  zigpc_protctrl_nwmgmt_write.cpp
  DEPENDS
  zigpc_common_mock
  uic_mqtt_mock
  zigpc_protctrl_nwmgmt_write_mock
  zigpc_net_mgmt_mock
  uic_contiki_stub)

# NWMGMT Write Tests
target_add_unittest(
  zigpc_protctrl
  NAME
  zigpc_protctrl_nwmgmt_write_test
  SOURCES
  zigpc_protctrl_nwmgmt_write_test.c
  EXCLUDE
  zigpc_protctrl_nwmgmt.c
  zigpc_protctrl_process.c
  zigpc_protctrl_nwmgmt_write_callback.c
  zigpc_protctrl_observers.c
  DEPENDS
  zigpc_common_mock
  zigpc_protctrl_nwmgmt_write_callback_mock
  zigpc_protctrl_process_send_mock
  zigpc_net_mgmt_mock
  uic_mqtt_mock
  uic_contiki_stub)

# NWMGMT Write Callback Tests
target_add_unittest(
  zigpc_protctrl
  NAME
  zigpc_protctrl_nwmgmt_write_callback_test
  SOURCES
  zigpc_protctrl_nwmgmt_write_callback_test.c
  DEPENDS
  zigpc_net_mgmt_mock
  zigpc_common_mock)
