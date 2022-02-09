/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

//ZPC components
#include "sl_log.h"
#include "contiki_test_helper.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute.hpp"
#include "datastore.h"
#include "attribute_store_fixt.h"
#include "zwave_unid.h"
#include <zpc_attribute_store.h>  // for get_zpc_network_node
// This module
#include "failing_node_monitor.h"

#define LOG_TAG "failing_node_monitor_test"

using namespace attribute_store;
attribute n5;
attribute n1;
attribute n2;
attribute n3;
attribute n4;
extern "C" {
#include "zwave_tx_mock.h"
#include "zwave_network_management_mock.h"
/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

zwave_operating_mode_t zwave_get_operating_mode(zwave_node_id_t node_id)
{
  if (node_id == 0xff) {
    return OPERATING_MODE_NL;
  }
  if (node_id == 6) {
    return OPERATING_MODE_FL;
  }
  return OPERATING_MODE_AL;
}

zwave_home_id_t my_zwave_network_management_get_home_id(int i)
{
  zwave_home_id_t home_id = 0xDEADBEEF;
  return home_id;
}

zwave_node_id_t my_zwave_network_management_get_node_id(int i)
{
  return 1;
}

clock_time_t read_node_failing_node_ping_interval_from_attribute_store(
  zwave_node_id_t node_id_to_test)
{

  //Read the home_id for ZPC network
  attribute network = get_zpc_network_node();
  for (auto zw_node: network.children(ATTRIBUTE_NODE_ID)) {
    try {
      zwave_node_id_t node_id = zw_node.reported<zwave_node_id_t>();
      if (node_id != node_id_to_test) {
        continue;
      }

      clock_time_t interval
        = zw_node.child_by_type(ATTRIBUTE_ZWAVE_FAILING_NODE_PING_INTERVAL)
            .reported<clock_time_t>();
      return interval;
    } catch (const std::invalid_argument &) {
    }
  }
  return 0;
}

void setUp()
{
  contiki_test_helper_init();
  datastore_init(":memory:");
  attribute_store_init();

  zwave_network_management_get_home_id_Stub(
    my_zwave_network_management_get_home_id);
  zwave_network_management_get_node_id_Stub(
    my_zwave_network_management_get_node_id);
  //This is ugly....
  zwave_unid_set_home_id(0xDEADBEEF);
  n1 = attribute::root()
         .emplace_node<zwave_home_id_t>(ATTRIBUTE_HOME_ID, 0xDEADBEEF)
         .emplace_node<zwave_node_id_t>(ATTRIBUTE_NODE_ID, 1);

  n5 = attribute::root()
         .emplace_node<zwave_home_id_t>(ATTRIBUTE_HOME_ID, 0xDEADBEEF)
         .emplace_node<zwave_node_id_t>(ATTRIBUTE_NODE_ID, 5);

  n5.emplace_node<clock_time_t>(ATTRIBUTE_ZWAVE_FAILING_NODE_PING_INTERVAL,
                                400000);
  n2 = attribute::root()
         .emplace_node<zwave_home_id_t>(ATTRIBUTE_HOME_ID, 0xDEADBEEF)
         .emplace_node<zwave_node_id_t>(ATTRIBUTE_NODE_ID, 2);
  n3 = attribute::root()
         .emplace_node<zwave_home_id_t>(ATTRIBUTE_HOME_ID, 0xDEADBEEF)
         .emplace_node<zwave_node_id_t>(ATTRIBUTE_NODE_ID, 3);
  n4 = attribute::root()
         .emplace_node<zwave_home_id_t>(ATTRIBUTE_HOME_ID, 0xDEADBEEF)
         .emplace_node<zwave_node_id_t>(ATTRIBUTE_NODE_ID, 6);

  failing_node_monitor_list_load();

  attribute_store_log_node(attribute::root(), true);
}

void tearDown()
{
  attribute_store_teardown();
  datastore_teardown();
}

static on_zwave_tx_send_data_complete_t my_zwave_tx_send_data_cb;
sl_status_t my_zwave_tx_send_data_stub(
  const zwave_controller_connection_info_t *connection,
  uint16_t data_length,
  const uint8_t *data,
  const zwave_tx_options_t *tx_options,
  const on_zwave_tx_send_data_complete_t on_send_complete,
  void *user,
  zwave_tx_session_id_t *session,
  int cmock_num_calls)
{
  my_zwave_tx_send_data_cb = on_send_complete;
  return SL_STATUS_OK;
}

void test_node_2_failing_and_online()
{
  uint8_t nop_frame[1]                               = {0};
  zwave_controller_connection_info_t connection_info = {};
  connection_info.remote.node_id                     = 2;
  connection_info.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  zwave_tx_options_t tx_options = {0};
  tx_options.qos_priority       = ZWAVE_TX_QOS_MIN_PRIORITY;
  attribute n2;
  n2 = attribute::root().emplace_node<zwave_node_id_t>(ATTRIBUTE_NODE_ID, 2);
  zwave_tx_send_data_ExpectWithArrayAndReturn(&connection_info,
                                              sizeof(connection_info),
                                              sizeof(nop_frame),
                                              nop_frame,
                                              sizeof(nop_frame),
                                              &tx_options,
                                              sizeof(tx_options),
                                              0,
                                              0,
                                              sizeof(void *),
                                              0,
                                              sizeof(zwave_tx_session_id_t *),
                                              SL_STATUS_OK);
  start_monitoring_failing_node(2);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(2),
    4000);
  contiki_test_helper_run(4000);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(2),
    8000);
  stop_monitoring_failing_node(
    2);  //emulate that network monitor detected that node 2 is alive now as NOP was delivered
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(2),
    0);
}

void test_node_2_failing_online_node_3_failing_online()
{
  uint8_t nop_frame[1]                               = {0};
  zwave_controller_connection_info_t connection_info = {};
  connection_info.remote.node_id                     = 2;
  connection_info.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  zwave_tx_options_t tx_options = {0};
  tx_options.qos_priority       = ZWAVE_TX_QOS_MIN_PRIORITY;
  zwave_tx_send_data_ExpectWithArrayAndReturn(&connection_info,
                                              sizeof(connection_info),
                                              sizeof(nop_frame),
                                              nop_frame,
                                              sizeof(nop_frame),
                                              &tx_options,
                                              sizeof(tx_options),
                                              0,
                                              0,
                                              sizeof(void *),
                                              0,
                                              sizeof(zwave_tx_session_id_t *),
                                              SL_STATUS_OK);
  start_monitoring_failing_node(2);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(2),
    4000);
  contiki_test_helper_run(4000);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(2),
    8000);
  stop_monitoring_failing_node(
    2);  //emulate that network monitor detected that node 2 is alive now as NOP was delivered
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(2),
    0);
  connection_info.remote.node_id = 3;
  zwave_tx_send_data_ExpectWithArrayAndReturn(&connection_info,
                                              sizeof(connection_info),
                                              sizeof(nop_frame),
                                              nop_frame,
                                              sizeof(nop_frame),
                                              &tx_options,
                                              sizeof(tx_options),
                                              0,
                                              0,
                                              sizeof(void *),
                                              0,
                                              sizeof(zwave_tx_session_id_t *),
                                              SL_STATUS_OK);
  start_monitoring_failing_node(3);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(3),
    4000);
  contiki_test_helper_run(4000);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(3),
    8000);
}

void test_node_2_failing_failing_online()
{
  uint8_t nop_frame[1] = {0};

  zwave_controller_connection_info_t connection_info2 = {};
  connection_info2.remote.node_id                     = 2;
  connection_info2.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  zwave_tx_options_t tx_options  = {0};
  tx_options.qos_priority        = ZWAVE_TX_QOS_MIN_PRIORITY;
  zwave_tx_send_data_ExpectWithArrayAndReturn(&connection_info2,
                                              sizeof(connection_info2),
                                              sizeof(nop_frame),
                                              nop_frame,
                                              sizeof(nop_frame),
                                              &tx_options,
                                              sizeof(tx_options),
                                              0,
                                              0,
                                              sizeof(void *),
                                              0,
                                              sizeof(zwave_tx_session_id_t *),
                                              SL_STATUS_OK);
  start_monitoring_failing_node(2);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(2),
    4000);
  contiki_test_helper_run(4000);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(2),
    8000);

  zwave_tx_send_data_ExpectWithArrayAndReturn(&connection_info2,
                                              sizeof(connection_info2),
                                              sizeof(nop_frame),
                                              nop_frame,
                                              sizeof(nop_frame),
                                              &tx_options,
                                              sizeof(tx_options),
                                              0,
                                              0,
                                              sizeof(void *),
                                              0,
                                              sizeof(zwave_tx_session_id_t *),
                                              SL_STATUS_OK);

  start_monitoring_failing_node(2);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(2),
    4000);
  contiki_test_helper_run(8000);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(2),
    8000);
}

void test_node_2_failing_3_failing_3_online_2_online()
{
  uint8_t nop_frame[1] = {0};

  zwave_controller_connection_info_t connection_info2 = {};
  connection_info2.remote.node_id                     = 2;
  connection_info2.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;

  zwave_tx_options_t tx_options = {0};
  tx_options.qos_priority       = ZWAVE_TX_QOS_MIN_PRIORITY;
  zwave_tx_send_data_ExpectWithArrayAndReturn(&connection_info2,
                                              sizeof(connection_info2),
                                              sizeof(nop_frame),
                                              nop_frame,
                                              sizeof(nop_frame),
                                              &tx_options,
                                              sizeof(tx_options),
                                              0,
                                              0,
                                              sizeof(void *),
                                              0,
                                              sizeof(zwave_tx_session_id_t *),
                                              SL_STATUS_OK);
  start_monitoring_failing_node(2);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(2),
    4000);
  contiki_test_helper_run(4000);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(2),
    8000);

  connection_info2.remote.node_id = 3;

  zwave_tx_send_data_ExpectWithArrayAndReturn(&connection_info2,
                                              sizeof(connection_info2),
                                              sizeof(nop_frame),
                                              nop_frame,
                                              sizeof(nop_frame),
                                              &tx_options,
                                              sizeof(tx_options),
                                              0,
                                              0,
                                              sizeof(void *),
                                              0,
                                              sizeof(zwave_tx_session_id_t *),
                                              SL_STATUS_OK);
  start_monitoring_failing_node(3);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(3),
    4000);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(2),
    8000);
  contiki_test_helper_run(4000);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(3),
    8000);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(2),
    4000);
  stop_monitoring_failing_node(3);
  print_failing_node_monitor_list();
  connection_info2.remote.node_id = 2;
  zwave_tx_send_data_ExpectWithArrayAndReturn(&connection_info2,
                                              sizeof(connection_info2),
                                              sizeof(nop_frame),
                                              nop_frame,
                                              sizeof(nop_frame),
                                              &tx_options,
                                              sizeof(tx_options),
                                              0,
                                              0,
                                              sizeof(void *),
                                              0,
                                              sizeof(zwave_tx_session_id_t *),
                                              SL_STATUS_OK);

  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(3),
    0);
  contiki_test_helper_run(4000);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(2),
    16000);
  stop_monitoring_failing_node(2);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(2),
    0);
  contiki_test_helper_run(4000);
  sl_log_debug(LOG_TAG, "------------");
}

void test_node_NL()
{
  start_monitoring_failing_node(0xff);
}
void test_node_freq_listening_failing_and_online()
{
  uint8_t nop_frame[1]                               = {0};
  zwave_controller_connection_info_t connection_info = {};
  connection_info.remote.node_id                     = 6;
  connection_info.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  zwave_tx_options_t tx_options = {0};
  tx_options.qos_priority       = ZWAVE_TX_QOS_MIN_PRIORITY;
  attribute n2;
  n2 = attribute::root().emplace_node<zwave_node_id_t>(ATTRIBUTE_NODE_ID, 2);
  zwave_tx_send_data_ExpectWithArrayAndReturn(&connection_info,
                                              sizeof(connection_info),
                                              sizeof(nop_frame),
                                              nop_frame,
                                              sizeof(nop_frame),
                                              &tx_options,
                                              sizeof(tx_options),
                                              0,
                                              0,
                                              sizeof(void *),
                                              0,
                                              sizeof(zwave_tx_session_id_t *),
                                              SL_STATUS_OK);
  start_monitoring_failing_node(6);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(6),
    40000);
  contiki_test_helper_run(40000);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(6),
    160000);
  stop_monitoring_failing_node(
    6);  //emulate that network monitor detected that node 2 is alive now as NOP was delivered
}

void test_contiki_overflow_node_2_failing_and_online()
{
  uint8_t nop_frame[1]                               = {0};
  clock_time_t init = -1;
  zwave_controller_connection_info_t connection_info = {};
  connection_info.remote.node_id                     = 2;
  connection_info.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  zwave_tx_options_t tx_options = {0};
  tx_options.qos_priority       = ZWAVE_TX_QOS_MIN_PRIORITY;
  stop_monitoring_failing_node(5);
  contiki_test_helper_run(init);
  zwave_tx_send_data_ExpectWithArrayAndReturn(&connection_info,
                                              sizeof(connection_info),
                                              sizeof(nop_frame),
                                              nop_frame,
                                              sizeof(nop_frame),
                                              &tx_options,
                                              sizeof(tx_options),
                                              0,
                                              0,
                                              sizeof(void *),
                                              0,
                                              sizeof(zwave_tx_session_id_t *),
                                              SL_STATUS_OK);
  start_monitoring_failing_node(2);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(2),
    4000);
  contiki_test_helper_run(4000);
  zwave_tx_send_data_ExpectWithArrayAndReturn(&connection_info,
                                              sizeof(connection_info),
                                              sizeof(nop_frame),
                                              nop_frame,
                                              sizeof(nop_frame),
                                              &tx_options,
                                              sizeof(tx_options),
                                              0,
                                              0,
                                              sizeof(void *),
                                              0,
                                              sizeof(zwave_tx_session_id_t *),
                                              SL_STATUS_OK);

  contiki_test_helper_run(4000);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(2),
    8000);
  contiki_test_helper_run(4000);
  TEST_ASSERT_EQUAL(
    read_node_failing_node_ping_interval_from_attribute_store(2),
    16000);
  contiki_test_helper_run(8000);

  stop_monitoring_failing_node(
    2);  //emulate that network monitor detected that node 2 is alive now as NOP was delivered
}
}
