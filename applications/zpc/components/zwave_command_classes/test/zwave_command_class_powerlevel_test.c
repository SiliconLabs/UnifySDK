/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include "zwave_command_class_powerlevel.h"
#include "zwave_command_class_test_helper.h"

// Includes from other components
#include "zwave_tx_mock.h"
#include "zwave_command_handler_mock.h"
#include "zwave_controller_connection_info.h"
#include "ZW_classcmd.h"

// Test includes
#include "unity.h"
#include "zwapi_protocol_basis.h"
#include "zwapi_protocol_basis_mock.h"
#include "zwapi_init_mock.h"
#include "contiki_test_helper.h"

on_zwave_tx_send_data_complete_t my_power_level_test_callback;
static sl_status_t save_my_power_level_test_callback(
  zwave_node_id_t destination_node_id,
  rf_power_level_t power_level,
  const on_zwave_tx_send_data_complete_t on_send_complete,
  void *user,
  zwave_tx_session_id_t *session,
  int cmock_num_calls)
{
  my_power_level_test_callback = on_send_complete;
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_CALLBACK(
  zwave_command_handler_t actual, int cmock_num_calls)
{
  zwave_command_handler_t expected = {0};
  expected.minimal_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME;
  expected.command_class      = COMMAND_CLASS_POWERLEVEL;
  expected.command_class_name = "Powerlevel";
  expected.version            = POWERLEVEL_VERSION;
  expected.support_handler    = &zwave_command_class_powerlevel_support_handler;
  expected.control_handler    = NULL;

  TEST_ASSERT_EQUAL_PTR(expected.support_handler, actual.support_handler);
  TEST_ASSERT_EQUAL_PTR(expected.control_handler, actual.control_handler);
  TEST_ASSERT_EQUAL(expected.minimal_scheme, actual.minimal_scheme);
  TEST_ASSERT_EQUAL(expected.command_class, actual.command_class);
  TEST_ASSERT_EQUAL_STRING(expected.command_class_name,
                           actual.command_class_name);
  TEST_ASSERT_EQUAL(expected.version, actual.version);
  TEST_ASSERT_FALSE(actual.manual_security_validation);
  return SL_STATUS_OK;
}

static void zwave_command_class_powerlevel_init_verifiation()
{
  zwave_command_handler_register_handler_AddCallback(
    zwave_command_handler_register_handler_CALLBACK);
  zwave_command_handler_register_handler_IgnoreAndReturn(SL_STATUS_OK);
  zwapi_set_rf_power_level_ExpectAndReturn(NORMAL_POWER, SL_STATUS_OK);
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Called before each and every test
void setUp()
{
  // Init before any test so ongoing powerlevel tests are wiped
  zwave_command_class_powerlevel_init_verifiation();
  zwave_command_class_powerlevel_init();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

// try to test node set powerelvel to more than MINIMUM_POWER
void test_zwave_command_class_powerlevel_test_node_set_more_than_minimum_power()
{
  const uint8_t cmd_frame_powerlevel_test_node_set_command[]
    = {COMMAND_CLASS_POWERLEVEL,
       POWERLEVEL_TEST_NODE_SET,
       42,
       MINIMUM_POWER
         + 1,  // try to test node set powerelvel to more than MINIMUM_POWER
       0,
       1};

  execute_frame_expect_result(
    zwave_command_class_powerlevel_support_handler,
    cmd_frame_powerlevel_test_node_set_command,
    sizeof(cmd_frame_powerlevel_test_node_set_command),
    SL_STATUS_FAIL);
}

// test short frame for POWERLEVEL_TEST_NODE_SET
void test_zwave_command_class_powerlevel_test_node_set_short_frame()
{
  const uint8_t cmd_frame_powerlevel_test_node_set_command[]
    = {COMMAND_CLASS_POWERLEVEL, POWERLEVEL_TEST_NODE_SET, 1};

  execute_frame_expect_result(
    zwave_command_class_powerlevel_support_handler,
    cmd_frame_powerlevel_test_node_set_command,
    sizeof(cmd_frame_powerlevel_test_node_set_command),
    SL_STATUS_NOT_SUPPORTED);
}
/* This tests POWERLEVEL_TEST_NODE_SET with failing zwapi_send_test_frame
 */
void test_zwave_command_class_powerlevel_test_node_set_fail_zwapi_send_test_frame()
{
  const uint8_t cmd_frame_powerlevel_test_node_set_command[]
    = {COMMAND_CLASS_POWERLEVEL,
       POWERLEVEL_TEST_NODE_SET,
       42,           //testNodeid
       MINUS_6_DBM,  //powerLevel
       0,
       1};

  zwave_tx_send_test_frame_ExpectAndReturn(42,
                                           MINUS_6_DBM,
                                           0,
                                           0,
                                           0,
                                           SL_STATUS_OK);
  zwave_tx_send_test_frame_IgnoreArg_on_send_complete();
  zwave_tx_send_test_frame_IgnoreArg_user();
  zwave_tx_send_test_frame_IgnoreArg_session_id();
  execute_frame_expect_result(
    zwave_command_class_powerlevel_support_handler,
    cmd_frame_powerlevel_test_node_set_command,
    sizeof(cmd_frame_powerlevel_test_node_set_command),
    SL_STATUS_OK);
}

/* This tests POWERLEVEL_TEST_NODE_SET
 */
void test_zwave_command_class_powerlevel_test_node_set()
{
  const uint8_t cmd_frame_powerlevel_test_node_set_command[]
    = {COMMAND_CLASS_POWERLEVEL,
       POWERLEVEL_TEST_NODE_SET,
       42,
       MINUS_6_DBM,
       0,
       2};

  zwave_tx_send_test_frame_AddCallback(save_my_power_level_test_callback);
  zwave_tx_send_test_frame_ExpectAndReturn(42,
                                           MINUS_6_DBM,
                                           0,
                                           0,
                                           0,
                                           SL_STATUS_OK);
  zwave_tx_send_test_frame_IgnoreArg_on_send_complete();
  zwave_tx_send_test_frame_IgnoreArg_user();
  zwave_tx_send_test_frame_IgnoreArg_session_id();

  execute_frame_expect_result(
    zwave_command_class_powerlevel_support_handler,
    cmd_frame_powerlevel_test_node_set_command,
    sizeof(cmd_frame_powerlevel_test_node_set_command),
    SL_STATUS_OK);

  // Next test frame is sent when we invoke the callback
  zwave_tx_send_test_frame_ExpectAndReturn(42,
                                           MINUS_6_DBM,
                                           0,
                                           0,
                                           0,
                                           SL_STATUS_OK);
  zwave_tx_send_test_frame_IgnoreArg_on_send_complete();
  zwave_tx_send_test_frame_IgnoreArg_user();
  zwave_tx_send_test_frame_IgnoreArg_session_id();
  my_power_level_test_callback(TRANSMIT_COMPLETE_OK, NULL, NULL);

  // No more test frame. callback will trigger the unsolicited report
  const uint8_t expected_frame[] = {COMMAND_CLASS_POWERLEVEL,
                                    POWERLEVEL_TEST_NODE_REPORT,
                                    42,
                                    TEST_SUCCESSFUL,
                                    0,
                                    2};

  expect_report_frame(expected_frame, sizeof(expected_frame));
  my_power_level_test_callback(TRANSMIT_COMPLETE_OK, NULL, NULL);
}

/* This test sends POWERLEVEL_TEST_NODE_SET and calls POWERLEVEL_TEST_NODE_GET
 * before the callback of zwapi_send_test_frame
 */
void test_zwave_command_class_powerlevel_test_node_set_and_get()
{
  const uint8_t cmd_frame_powerlevel_test_node_set_command[]
    = {COMMAND_CLASS_POWERLEVEL,
       POWERLEVEL_TEST_NODE_SET,
       42,
       MINUS_6_DBM,
       0,
       1};

  zwave_tx_send_test_frame_ExpectAndReturn(42,
                                           MINUS_6_DBM,
                                           0,
                                           0,
                                           0,
                                           SL_STATUS_OK);
  zwave_tx_send_test_frame_IgnoreArg_on_send_complete();
  zwave_tx_send_test_frame_IgnoreArg_user();
  zwave_tx_send_test_frame_IgnoreArg_session_id();
  zwave_tx_send_test_frame_AddCallback(save_my_power_level_test_callback);

  execute_frame_expect_result(
    zwave_command_class_powerlevel_support_handler,
    cmd_frame_powerlevel_test_node_set_command,
    sizeof(cmd_frame_powerlevel_test_node_set_command),
    SL_STATUS_OK);

  const uint8_t cmd_frame_powerlevel_get_command[]
    = {COMMAND_CLASS_POWERLEVEL, POWERLEVEL_TEST_NODE_GET};

  const uint8_t exp_frame_powerlevel_report_command[]
    = {COMMAND_CLASS_POWERLEVEL,
       POWERLEVEL_TEST_NODE_REPORT,
       42,
       POWERLEVEL_TEST_NODE_REPORT_ZW_TEST_INPROGRESS,
       0,
       0};

  execute_frame_expect_frame(zwave_command_class_powerlevel_support_handler,
                             cmd_frame_powerlevel_get_command,
                             sizeof(cmd_frame_powerlevel_get_command),
                             exp_frame_powerlevel_report_command,
                             sizeof(exp_frame_powerlevel_report_command),
                             SL_STATUS_OK);
}

/* This test sends POWERLEVEL_SET with timeout 100seconds and will send
 * POWERLEVEL GET to check the timeout value
 */
void test_zwave_command_class_powerlevel_set_get_timeout()
{
  const uint8_t cmd_frame_powerlevel_set_command[]
    = {COMMAND_CLASS_POWERLEVEL, POWERLEVEL_SET, MINUS_6_DBM, 100};

  zwapi_set_rf_power_level_ExpectAndReturn(MINUS_6_DBM, MINUS_6_DBM);
  execute_frame_expect_result(zwave_command_class_powerlevel_support_handler,
                              cmd_frame_powerlevel_set_command,
                              sizeof(cmd_frame_powerlevel_set_command),
                              SL_STATUS_OK);
  const uint8_t cmd_frame_powerlevel_get_command[]
    = {COMMAND_CLASS_POWERLEVEL, POWERLEVEL_GET};

  const uint8_t exp_frame_powerlevel_report_command[]
    = {COMMAND_CLASS_POWERLEVEL, POWERLEVEL_REPORT, MINUS_6_DBM, 100};

  execute_frame_expect_frame(zwave_command_class_powerlevel_support_handler,
                             cmd_frame_powerlevel_get_command,
                             sizeof(cmd_frame_powerlevel_get_command),
                             exp_frame_powerlevel_report_command,
                             sizeof(exp_frame_powerlevel_report_command),
                             SL_STATUS_OK);
}

/* This test sends POWERLEVEL_SET and will check that the return value is
 * SL_STATUS_OK
 */
void test_zwave_command_class_powerlevel_set()
{
  const uint8_t cmd_frame_powerlevel_command[]
    = {COMMAND_CLASS_POWERLEVEL, POWERLEVEL_SET, MINUS_6_DBM, 100};

  zwapi_set_rf_power_level_ExpectAndReturn(MINUS_6_DBM, MINUS_6_DBM);
  contiki_test_helper_run(100 * CLOCK_SECOND);
  execute_frame_expect_result(zwave_command_class_powerlevel_support_handler,
                              cmd_frame_powerlevel_command,
                              sizeof(cmd_frame_powerlevel_command),
                              SL_STATUS_OK);
}

/* This test sends shorter frame of POWERLEVEL_SET and will send
 * POWERLEVEL GET to check that the return value is SL_STATUS_OK
 */
void test_zwave_command_class_powerlevel_set_short_frame_length()
{
  const uint8_t cmd_frame_powerlevel_command[]
    = {COMMAND_CLASS_POWERLEVEL, POWERLEVEL_SET, 0};
  execute_frame_expect_result(zwave_command_class_powerlevel_support_handler,
                              cmd_frame_powerlevel_command,
                              sizeof(cmd_frame_powerlevel_command),
                              SL_STATUS_NOT_SUPPORTED);
}

//test sending timeout as 0 in POWERLEVEL_SET
void test_zwave_command_class_powerlevel_set_timeout_0()
{
  const uint8_t cmd_frame_powerlevel_command[]
    = {COMMAND_CLASS_POWERLEVEL, POWERLEVEL_SET, MINUS_6_DBM, 0};
  execute_frame_expect_result(zwave_command_class_powerlevel_support_handler,
                              cmd_frame_powerlevel_command,
                              sizeof(cmd_frame_powerlevel_command),
                              SL_STATUS_FAIL);
}

//test setting powerlevel to NORMAL_POWER on 500 series
void test_zwave_command_class_powerlevel_set_normal_power()
{
  const uint8_t cmd_frame_powerlevel_command[]
    = {COMMAND_CLASS_POWERLEVEL, POWERLEVEL_SET, NORMAL_POWER, 100};

  zwapi_set_rf_power_level_ExpectAndReturn(NORMAL_POWER, NORMAL_POWER);
  execute_frame_expect_result(zwave_command_class_powerlevel_support_handler,
                              cmd_frame_powerlevel_command,
                              sizeof(cmd_frame_powerlevel_command),
                              SL_STATUS_OK);
}

//test sending no command in powerlevel CC
void test_zwave_command_class_powerlevel_no_command()
{
  const uint8_t cmd_frame_powerlevel_command[] = {COMMAND_CLASS_POWERLEVEL};
  execute_frame_expect_result(zwave_command_class_powerlevel_support_handler,
                              cmd_frame_powerlevel_command,
                              sizeof(cmd_frame_powerlevel_command),
                              SL_STATUS_NOT_SUPPORTED);
}

//Test sending invalid command in powerlevel CC
void test_zwave_command_class_powerlevel_unknown_command()
{
  const uint8_t cmd_frame_powerlevel_command[]
    = {COMMAND_CLASS_POWERLEVEL, 0xFF, MINIMUM_POWER + 1, 100};
  execute_frame_expect_result(zwave_command_class_powerlevel_support_handler,
                              cmd_frame_powerlevel_command,
                              sizeof(cmd_frame_powerlevel_command),
                              SL_STATUS_NOT_SUPPORTED);
}

//Test setting powerlevel to more than MINIMUM_POWER
void test_zwave_command_class_powerlevel_set_more_than_MINUS_6_DBMminimum_power()
{
  const uint8_t cmd_frame_powerlevel_command[]
    = {COMMAND_CLASS_POWERLEVEL, POWERLEVEL_SET, MINIMUM_POWER + 1, 100};
  execute_frame_expect_result(zwave_command_class_powerlevel_support_handler,
                              cmd_frame_powerlevel_command,
                              sizeof(cmd_frame_powerlevel_command),
                              SL_STATUS_FAIL);
}

//Test powerelvel get on 500 series chip
void test_zwave_command_class_powerlevel_get_500()
{
  const uint8_t cmd_frame_powerlevel_command[]
    = {COMMAND_CLASS_POWERLEVEL, POWERLEVEL_GET};

  const uint8_t exp_frame_powerlevel_command[]
    = {COMMAND_CLASS_POWERLEVEL, POWERLEVEL_REPORT, NORMAL_POWER, 0};

  execute_frame_expect_frame(zwave_command_class_powerlevel_support_handler,
                             cmd_frame_powerlevel_command,
                             sizeof(cmd_frame_powerlevel_command),
                             exp_frame_powerlevel_command,
                             sizeof(exp_frame_powerlevel_command),
                             SL_STATUS_OK);
}

//Test powerelvel get on 700 series chip
void test_zwave_command_class_powerlevel_get_700()
{
  const uint8_t cmd_frame_powerlevel_command[]
    = {COMMAND_CLASS_POWERLEVEL, POWERLEVEL_GET};

  const uint8_t exp_frame_powerlevel_command[]
    = {COMMAND_CLASS_POWERLEVEL, POWERLEVEL_REPORT, NORMAL_POWER, 0};

  execute_frame_expect_frame(zwave_command_class_powerlevel_support_handler,
                             cmd_frame_powerlevel_command,
                             sizeof(cmd_frame_powerlevel_command),
                             exp_frame_powerlevel_command,
                             sizeof(exp_frame_powerlevel_command),
                             SL_STATUS_OK);
}

//Test handler resistering
void test_zwave_command_class_powerlevel_init()
{
  zwave_command_class_powerlevel_init_verifiation();
  zwave_command_class_powerlevel_init();
}

void test_zwave_command_class_powerlevel_no_answer_to_multicast_get()
{
  zwave_controller_connection_info_t test_connection = {};

  // Receive as multicast
  test_connection.local.is_multicast = true;

  const uint8_t incoming_frame_1[]
    = {COMMAND_CLASS_POWERLEVEL, POWERLEVEL_TEST_NODE_GET};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_command_class_powerlevel_support_handler(&test_connection,
                                                   incoming_frame_1,
                                                   sizeof(incoming_frame_1)));

  const uint8_t incoming_frame_2[] = {COMMAND_CLASS_POWERLEVEL, POWERLEVEL_GET};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_command_class_powerlevel_support_handler(&test_connection,
                                                   incoming_frame_2,
                                                   sizeof(incoming_frame_2)));
}

void test_zwave_command_class_powerlevel_wrong_cc()
{
  zwave_controller_connection_info_t test_connection = {};

  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_ZWAVEPLUS_INFO, POWERLEVEL_TEST_NODE_GET};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    zwave_command_class_powerlevel_support_handler(&test_connection,
                                                   incoming_frame,
                                                   sizeof(incoming_frame)));
}