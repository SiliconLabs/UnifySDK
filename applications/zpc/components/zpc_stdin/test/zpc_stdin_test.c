/* # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 */

// Test framework
#include "unity.h"
#include "cmock.h"

// Unify Component
#include "uic_stdin_test.h"

// Own component
#include "zpc_stdin_command_handling.h"
#include "zpc_stdin_fixt.h"

// ZPC Components mocks
#include "zwave_network_management_mock.h"
#include "zwave_controller_callbacks_mock.h"
#include "zwave_controller_mock.h"
#include "zwave_controller_utils_mock.h"
#include "zwave_command_handler_mock.h"
#include "zwave_tx_mock.h"
#include "zwave_tx_scheme_selector_mock.h"
#include "zwave_utils.h"
#include "zwave_unid_mock.h"
#include "zwave_command_class_supervision_mock.h"
#include "zpc_attribute_store_network_helper_mock.h"
#include "attribute_store_mock.h"
#include "attribute_store_configuration_mock.h"
#include "attribute_store_helper_mock.h"
#include "attribute_store_type_registration_mock.h"
#include "attribute_resolver_mock.h"
#include "ucl_mqtt_node_interview_mock.h"
#include "zwave_tx_groups_mock.h"
#include "zwave_command_class_association_send_mock.h"
#include "zwave_command_class_firmware_update_mock.h"
#include "zwave_s2_keystore_mock.h"
#include "attribute_poll_mock.h"

// Standard
#include <stdbool.h>
#include <string.h>

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  zwave_controller_register_callbacks_IgnoreAndReturn(SL_STATUS_OK);
  // Setup CLI with ZPC application
  zpc_stdin_setup_fixt();
}

/// Called before each and every test
void SetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void test_zwave_set_default()
{
  zwave_controller_reset_Expect();
  sl_status_t state = uic_stdin_handle_command("zwave_set_default");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_callbacks()
{
  zwave_dsk_t dsk                                 = {0x00,
                     0x00,
                     0xad,
                     0xde,
                     0xef,
                     0xbe,
                     0x10,
                     0x01,
                     0xad,
                     0xde,
                     0xef,
                     0xbe,
                     0xee,
                     0xff,
                     0x01,
                     0x00};
  const zwave_controller_callbacks_t nm_callbacks = get_zpc_stdin_callbacks();
  nm_callbacks.on_keys_report(true, 0x87);
  nm_callbacks.on_dsk_report(2, dsk, 0x87);
}

void test_remove_failed()
{
  sl_status_t state;

  zwave_node_id_t node_id = 0;
  int length              = 50;
  char command[length];
  snprintf(command, length, "zwave_remove_failed %u", node_id);
  zwave_network_management_remove_failed_ExpectAndReturn(node_id, SL_STATUS_OK);
  state = uic_stdin_handle_command(command);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  // Giving wrong argument inputs
  state = uic_stdin_handle_command("zwave_remove_failed");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  // Giving wrong argument inputs
  state = uic_stdin_handle_command("zwave_remove_failed ab");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);
}

void test_zwave_tx()
{
  sl_status_t state;
  zwave_tx_scheme_get_node_connection_info_Ignore();
  zwave_tx_scheme_get_node_tx_options_Ignore();

  // Wrong input command
  state = uic_stdin_handle_command("zwave_tx not");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("zwave_tx 1,23avbpotdf");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("zwave_tx aaa,23947234");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("zwave_tx");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("zwave_tx 02,03,04,2501FF,3495");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  // Send 0xFF to a node id
  zwave_tx_send_data_IgnoreAndReturn(SL_STATUS_OK);
  state = uic_stdin_handle_command("zwave_tx 02,2501FF");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  // Send with Multi Channel
  zwave_tx_send_data_IgnoreAndReturn(SL_STATUS_OK);
  state = uic_stdin_handle_command("zwave_tx 02,03,04,2501FF");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  // invalid arguments
  state = uic_stdin_handle_command("zwave_tx ab,cd");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);
}

void test_zwave_tx_log()
{
  sl_status_t state;
  // Will always call with true and return OK
  zwave_tx_log_queue_Expect(true);
  state = uic_stdin_handle_command("zwave_tx_log");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  zwave_tx_log_queue_Expect(true);
  state = uic_stdin_handle_command("zwave_tx_log 435,4j,re09j4r,");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_remove_zwave_node()
{
  zwave_network_management_remove_node_ExpectAndReturn(SL_STATUS_OK);
  sl_status_t state = uic_stdin_handle_command("zwave_remove_node");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_add_zwave_node_stop()
{
  zwave_network_management_abort_ExpectAndReturn(SL_STATUS_OK);
  sl_status_t state
    = uic_stdin_handle_command("zwave_network_management_abort");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_add_zwave_node()
{
  zwave_network_management_add_node_ExpectAndReturn(SL_STATUS_OK);
  sl_status_t state = uic_stdin_handle_command("zwave_add_node");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_attribute_store_log()
{
  attribute_store_get_root_IgnoreAndReturn(ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_get_node_type_IgnoreAndReturn(1);
  attribute_store_get_storage_type_IgnoreAndReturn(UNKNOWN_STORAGE_TYPE);
  attribute_store_is_value_defined_IgnoreAndReturn(false);
  attribute_store_get_type_name_IgnoreAndReturn("Hest");
  sl_status_t state = uic_stdin_handle_command("attribute_store_log");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_store_node_t node_id = 1;
  zwave_unid_from_node_id_Ignore();
  attribute_store_network_helper_get_node_id_node_IgnoreAndReturn(node_id);
  attribute_store_get_node_attribute_value_IgnoreAndReturn(SL_STATUS_OK);
  attribute_store_get_node_child_count_ExpectAndReturn(node_id, 1);
  attribute_store_get_node_child_ExpectAndReturn(node_id,
                                                 0,
                                                 ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_get_node_child_count_ExpectAndReturn(node_id, 0);

  state = uic_stdin_handle_command("attribute_store_log 0");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("attribute_store_log FAIL");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);
}

void test_attribute_store_log_network()
{
  attribute_store_get_root_IgnoreAndReturn(ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_get_node_type_IgnoreAndReturn(1);
  attribute_store_get_storage_type_IgnoreAndReturn(UNKNOWN_STORAGE_TYPE);
  attribute_store_is_value_defined_IgnoreAndReturn(false);
  attribute_store_get_type_name_IgnoreAndReturn("Hest");
  sl_status_t state = uic_stdin_handle_command("attribute_store_log_network");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_store_node_t node_id = 1;
  zwave_unid_from_node_id_Ignore();
  attribute_store_network_helper_get_node_id_node_IgnoreAndReturn(node_id);
  attribute_store_get_node_attribute_value_IgnoreAndReturn(SL_STATUS_OK);
  attribute_store_get_node_child_count_ExpectAndReturn(node_id, 1);
  attribute_store_get_node_child_ExpectAndReturn(node_id,
                                                 0,
                                                 ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_get_node_child_count_ExpectAndReturn(node_id, 0);

  state = uic_stdin_handle_command("attribute_store_log 0");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("attribute_store_log NEXT");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);
}

void test_attribute_store_log_search()
{
  attribute_store_get_root_IgnoreAndReturn(ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_get_node_type_IgnoreAndReturn(1);
  attribute_store_get_storage_type_IgnoreAndReturn(UNKNOWN_STORAGE_TYPE);
  attribute_store_is_value_defined_IgnoreAndReturn(false);
  attribute_store_get_type_name_IgnoreAndReturn("Hest");
  sl_status_t state
    = uic_stdin_handle_command("attribute_store_log_search Hest");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_store_log_search");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);
}

void test_attribute_store_log_node()
{
  attribute_store_log_node_Ignore();
  sl_status_t state = uic_stdin_handle_command("attribute_store_log_node 1");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("attribute_store_log_node");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_store_log_node FAIL");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);
}

void test_attribute_store_resolve_attribute()
{
  attribute_store_set_node_attribute_value_IgnoreAndReturn(SL_STATUS_OK);
  sl_status_t state
    = uic_stdin_handle_command("attribute_store_resolve_attribute 1");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("attribute_store_resolve_attribute");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_store_resolve_attribute FAIL");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);
}

void test_grant_keys()
{
  sl_status_t state = uic_stdin_handle_command("zwave_grant_keys");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  zwave_network_management_keys_set_ExpectAndReturn(true,
                                                    false,
                                                    0xFF,
                                                    SL_STATUS_OK);
  state = uic_stdin_handle_command("zwave_grant_keys 1,0xFF");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  zwave_network_management_keys_set_ExpectAndReturn(true,
                                                    false,
                                                    0,
                                                    SL_STATUS_OK);
  state = uic_stdin_handle_command("zwave_grant_keys 1,0");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  zwave_network_management_keys_set_ExpectAndReturn(false,
                                                    false,
                                                    0xFF,
                                                    SL_STATUS_OK);
  state = uic_stdin_handle_command("zwave_grant_keys 0,0xFF");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  zwave_network_management_keys_set_ExpectAndReturn(true,
                                                    true,
                                                    0xFF,
                                                    SL_STATUS_OK);
  state = uic_stdin_handle_command("zwave_grant_keys 1,0xFF,1");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("zwave_grant_keys sadsada,sadasda");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);
}

void test_accept_dsk()
{
  sl_status_t state = uic_stdin_handle_command("zwave_accept_dsk");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  uint8_t node_reported_dsk[16] = {0};
  node_reported_dsk[1]          = (uint8_t)12;
  node_reported_dsk[0]          = (uint8_t)(12 >> 8);
  zwave_network_management_dsk_set_ExpectAndReturn(node_reported_dsk,
                                                   SL_STATUS_OK);
  state = uic_stdin_handle_command("zwave_accept_dsk 12");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_attribute_store_set_desired()
{
  sl_status_t state = uic_stdin_handle_command("attribute_store_set_desired");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_store_set_desired 34");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_store_set_desired 3fdsa083");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_store_set_desired 34,hello");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_store_set_desired_number_ExpectAndReturn(2394,
                                                     124,
                                                     SL_STATUS_NOT_READY);

  state = uic_stdin_handle_command("attribute_store_set_desired 2394,124");
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_READY, state);
}

void test_attribute_store_set_all_desired_types()
{
  sl_status_t state;
  state = uic_stdin_handle_command("attribute_store_set_all_desired_types");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_store_set_all_desired_types 34");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_store_get_root_IgnoreAndReturn(ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_set_node_attribute_value_IgnoreAndReturn(SL_STATUS_OK);
  attribute_store_get_node_child_count_IgnoreAndReturn(0);
  attribute_store_get_node_child_IgnoreAndReturn(ATTRIBUTE_STORE_INVALID_NODE);

  state
    = uic_stdin_handle_command("attribute_store_set_all_desired_types 34,49");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command(
    "attribute_store_set_all_desired_types 3adsæfkm4,34fdahhtn4553");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_attribute_store_set_reported()
{
  attribute_store_set_node_attribute_value_StopIgnore();

  sl_status_t state = uic_stdin_handle_command("attribute_store_set_reported");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_store_set_reported 34");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_store_set_reported 3fdsa083");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_store_set_reported 34,hello");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_store_set_reported_number_ExpectAndReturn(2394, 124, SL_STATUS_OK);

  state = uic_stdin_handle_command("attribute_store_set_reported 2394,124");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_zwave_wake_up()
{
  sl_status_t state = uic_stdin_handle_command("zwave_wake_up");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("zwave_wake_up 1,23,4,6");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("zwave_wake_up sioneqr");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  zwave_command_class_supervision_wake_on_demand_ExpectAndReturn(23,
                                                                 SL_STATUS_OK);
  state = uic_stdin_handle_command("zwave_wake_up 23");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_attribute_resolver_log()
{
  sl_status_t state;
  attribute_resolver_state_log_Expect();
  state = uic_stdin_handle_command("attribute_resolver_log");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  attribute_resolver_state_log_Expect();
  state = uic_stdin_handle_command("attribute_resolver_log 1,23,4,6");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_zwave_initiate_interview()
{
  zwave_unid_from_node_id_Ignore();
  sl_status_t state;
  state = uic_stdin_handle_command("zwave_initiate_interview");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("zwave_initiate_interview not");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  ucl_mqtt_initiate_node_interview_ExpectAndReturn(NULL, SL_STATUS_OK);
  ucl_mqtt_initiate_node_interview_IgnoreArg_node_unid();
  state = uic_stdin_handle_command("zwave_initiate_interview 2");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  ucl_mqtt_initiate_endpoint_interview_ExpectAndReturn(
    NULL,
    2,
    SL_STATUS_WOULD_OVERFLOW);
  ucl_mqtt_initiate_endpoint_interview_IgnoreArg_node_unid();
  state = uic_stdin_handle_command("zwave_initiate_interview 2,2");
  TEST_ASSERT_EQUAL(SL_STATUS_WOULD_OVERFLOW, state);

  ucl_mqtt_initiate_endpoint_interview_ExpectAndReturn(NULL, 2, SL_STATUS_OK);
  ucl_mqtt_initiate_endpoint_interview_IgnoreArg_node_unid();
  state = uic_stdin_handle_command(
    "zwave_initiate_interview 2,2,some_ignored_argument");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_zwave_tx_multi()
{
  zwave_tx_scheme_get_node_connection_info_Ignore();
  zwave_tx_scheme_get_node_tx_options_Ignore();

  sl_status_t state;
  state = uic_stdin_handle_command("zwave_tx_multi");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("zwave_tx_multi 34");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  zwave_tx_assign_group_IgnoreAndReturn(SL_STATUS_FAIL);
  state = uic_stdin_handle_command("zwave_tx_multi 34,49");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  zwave_tx_assign_group_IgnoreAndReturn(SL_STATUS_OK);
  zwave_command_class_supervision_send_data_IgnoreAndReturn(
    SL_STATUS_HAS_OVERFLOWED);
  state = uic_stdin_handle_command("zwave_tx_multi 34,3453");
  TEST_ASSERT_EQUAL(SL_STATUS_HAS_OVERFLOWED, state);

  state = uic_stdin_handle_command("zwave_tx_multi 3adsæfkm4,34fdahhtn4553");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);
}

void test_handle_cc_versions_log()
{
  zwave_command_handler_print_info_Expect(0);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_stdin_handle_command("zwave_cc_versions_log"));
}

void test_handle_zwave_s2_log_security_keys()
{
  zwave_s2_log_security_keys_Expect(1);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_stdin_handle_command("zwave_log_security_keys"));
}

void test_handle_zwave_save_security_keys_to_file()
{
  zwave_s2_save_security_keys_Expect("test.txt");
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_stdin_handle_command(
                      "zwave_save_security_keys_to_file test.txt"));
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_stdin_handle_command(
                      "zwave_save_security_keys_to_file"));
}

void test_zwave_tx_association()
{
  zwave_tx_scheme_get_node_connection_info_Ignore();
  zwave_tx_scheme_get_node_tx_options_Ignore();

  sl_status_t state;
  state = uic_stdin_handle_command("zwave_tx_association");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("zwave_tx_association 34");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  zwave_association_send_to_group_IgnoreAndReturn(SL_STATUS_OK);
  state = uic_stdin_handle_command("zwave_tx_association 34,49");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  zwave_association_send_to_group_IgnoreAndReturn(SL_STATUS_NOT_SUPPORTED);
  state = uic_stdin_handle_command("zwave_tx_association 34,49");
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED, state);

  state = uic_stdin_handle_command("zwave_tx_association qqqq,0344");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);
}

void test_zwave_initiate_firmware_update()
{
  sl_status_t state;
  state = uic_stdin_handle_command("zwave_initiate_firmware_update");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("zwave_initiate_firmware_update 34");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state
    = uic_stdin_handle_command("zwave_initiate_firmware_update zzz,test.gbl");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  command_class_firmware_update_initiate_firmware_update_ExpectAndReturn(
    34,
    0,
    0,
    0,
    "454fdsfda9",
    SL_STATUS_OK);
  command_class_firmware_update_initiate_firmware_update_IgnoreArg_apply_after();
  state
    = uic_stdin_handle_command("zwave_initiate_firmware_update 34,454fdsfda9");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_zwave_abort_firmware_update()
{
  sl_status_t state;
  state = uic_stdin_handle_command("zwave_abort_firmware_update");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("zwave_abort_firmware_update zzz");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  command_class_firmware_update_abort_ongoing_firmware_operation_Expect(34, 0);
  state = uic_stdin_handle_command(
    "zwave_abort_firmware_update 34,some_more_arguments");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  command_class_firmware_update_abort_ongoing_firmware_operation_Expect(300, 0);
  state = uic_stdin_handle_command("zwave_abort_firmware_update 300");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_zwave_learn_mode()
{
  sl_status_t state;
  state = uic_stdin_handle_command("zwave_learn_mode");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("zwave_learn_mode 1,2");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  zwave_network_management_learn_mode_ExpectAndReturn(
    ZWAVE_NETWORK_MANAGEMENT_LEARN_NWI,
    SL_STATUS_BUSY);
  state = uic_stdin_handle_command("zwave_learn_mode nwi");
  TEST_ASSERT_EQUAL(SL_STATUS_BUSY, state);

  zwave_network_management_learn_mode_ExpectAndReturn(
    ZWAVE_NETWORK_MANAGEMENT_LEARN_NONE,
    SL_STATUS_ABORT);
  state = uic_stdin_handle_command("zwave_learn_mode stop");
  TEST_ASSERT_EQUAL(SL_STATUS_ABORT, state);

  zwave_network_management_learn_mode_ExpectAndReturn(
    ZWAVE_NETWORK_MANAGEMENT_LEARN_NWE,
    SL_STATUS_FAIL);
  state = uic_stdin_handle_command("zwave_learn_mode nwe");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("zwave_learn_mode unknown");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("zwave_learn_mode ");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_zwave_command_handler_dispatch()
{
  sl_status_t state
    = uic_stdin_handle_command("zwave_command_handler_dispatch");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("zwave_command_handler_dispatch s");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  uint8_t expected_frame_data[] = {0xde, 0xad, 0xbe, 0xef, 0x42};
  zwave_network_management_get_node_id_ExpectAndReturn(0);
  zwave_controller_encapsulation_scheme_t zpc_scheme
    = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED;
  zpc_highest_security_class_ExpectAndReturn(zpc_scheme);
  zwave_command_handler_dispatch_ExpectAndReturn(NULL,
                                                 expected_frame_data,
                                                 sizeof(expected_frame_data),
                                                 SL_STATUS_OK);
  zwave_command_handler_dispatch_IgnoreArg_connection_info();
  state = uic_stdin_handle_command("zwave_command_handler_dispatch deadbeef42");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_attribute_poll_register()
{
  sl_status_t state = uic_stdin_handle_command("attribute_poll_register");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_poll_register fish");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_poll_register_ExpectAndReturn(42, 0, SL_STATUS_OK);
  state = uic_stdin_handle_command("attribute_poll_register 42");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  attribute_poll_register_ExpectAndReturn(1234, 42, SL_STATUS_OK);
  state = uic_stdin_handle_command("attribute_poll_register 1234,42");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_attribute_poll_deregister()
{
  sl_status_t state = uic_stdin_handle_command("attribute_poll_deregister");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_poll_deregister fish");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_poll_deregister_ExpectAndReturn(42, SL_STATUS_OK);
  state = uic_stdin_handle_command("attribute_poll_deregister 42");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}
