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
#include "unity.h"
#include "ucl_node_state.h"

// Includes from other components
#include "attribute_store_defined_attribute_types.h"

// Test helpers
#include "contiki_test_helper.h"

// Mock includes
#include "attribute_store_mock.h"
#include "attribute_store_helper_mock.h"
#include "zpc_attribute_store_mock.h"
#include "zpc_attribute_store_network_helper_mock.h"
#include "zwave_utils_mock.h"
#include "uic_mqtt_mock.h"
#include "ucl_mqtt_node_interview_mock.h"
#include "ucl_network_management_mock.h"
#include "dotdot_mqtt_mock.h"

// Interfaces
#include "ucl_definitions.h"
#include "zwave_controller_keyset.h"

// static functions intercept
static attribute_store_node_changed_callback_t node_id_update_callback_save;
static attribute_store_node_changed_callback_t
  network_status_update_callback_save;
static uic_mqtt_dotdot_state_interview_callback_t
  uic_mqtt_dotdot_state_interview_callback_save;
static uic_mqtt_dotdot_state_remove_offline_callback_t
  uic_mqtt_dotdot_state_remove_offline_callback_save;
static uic_mqtt_dotdot_state_discover_neighbors_callback_t
  uic_mqtt_dotdot_state_discover_neighbors_callback_save;

// Static variables
static attribute_store_node_t test_home_id_node;
static attribute_store_node_t test_node_id_node;
static zwave_node_id_t test_node_id;
static uint32_t delay_value;
static node_state_topic_state_t network_status;
static zwave_keyset_t granted_keys;
static unid_t test_unid = "zw-DEADBEEF-0009";

// The one attribute_store_node_t (NodeID) saved in our network
const attribute_store_node_t node_id_in_network_1 = 0x99;
const attribute_store_node_t node_id_in_network_2 = 0x25;

// Expected publications
const static char expected_state_topic[] = "ucl/by-unid/zw-DEADBEEF-0009/State";
const static char expected_state_message_1[]
  = "{\"NetworkStatus\": \"Online interviewing\", \"Security\": \"None\", "
    "\"MaximumCommandDelay\": \"infinite\"}";
const static char expected_state_message_2[]
  = "{\"NetworkStatus\": \"Online functional\", \"Security\": \"Z-Wave S2 "
    "Access "
    "Control\", "
    "\"MaximumCommandDelay\": \"unknown\"}";
const static char expected_state_message_3[]
  = "{\"NetworkStatus\": \"Unavailable\", \"Security\": \"Z-Wave S0\", "
    "\"MaximumCommandDelay\": 350}";

// Stub / intercept functions
static sl_status_t attribute_store_callback_registration_stub(
  attribute_store_node_changed_callback_t callback_function,
  attribute_store_type_t type,
  attribute_store_node_value_state_t value_state,
  int cmock_num_calls);

// Stub / intercept functions for uic_mqtt_dotdot_state_interview_callback_set
static void uic_mqtt_dotdot_state_interview_callback_set_stub(
  const uic_mqtt_dotdot_state_interview_callback_t callback_type,
  int cmock_num_calls);

// Stub / intercept functions for uic_mqtt_dotdot_state_remove_offline_callback_set
static void uic_mqtt_dotdot_state_remove_offline_callback_set_stub(
  const uic_mqtt_dotdot_state_remove_offline_callback_t callback_type,
  int cmock_num_calls);

// Stub / intercept functions for uic_mqtt_dotdot_state_discover_neighbors_callback_set
static void uic_mqtt_dotdot_state_discover_neighbors_callback_set_stub(
  const uic_mqtt_dotdot_state_discover_neighbors_callback_t callback_type,
  int cmock_num_calls);

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  node_id_update_callback_save                           = NULL;
  network_status_update_callback_save                    = NULL;
  uic_mqtt_dotdot_state_interview_callback_save          = NULL;
  uic_mqtt_dotdot_state_remove_offline_callback_save     = NULL;
  uic_mqtt_dotdot_state_discover_neighbors_callback_save = NULL;
  test_home_id_node = ATTRIBUTE_STORE_INVALID_NODE;
  test_node_id_node = ATTRIBUTE_STORE_INVALID_NODE;
  test_node_id      = 0;
  delay_value       = 0;
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  contiki_test_helper_init();
  zwave_unid_set_home_id(0xDEADBEEF);

  attribute_store_register_callback_by_type_and_state_Stub(
    attribute_store_callback_registration_stub);
  uic_mqtt_dotdot_state_interview_callback_set_Stub(
    uic_mqtt_dotdot_state_interview_callback_set_stub);
  uic_mqtt_dotdot_state_remove_offline_callback_set_Stub(
    uic_mqtt_dotdot_state_remove_offline_callback_set_stub);
  uic_mqtt_dotdot_state_discover_neighbors_callback_set_Stub(
    uic_mqtt_dotdot_state_discover_neighbors_callback_set_stub);

  attribute_store_register_callback_by_type_and_state_ExpectAndReturn(
    NULL,
    ATTRIBUTE_NODE_ID,
    REPORTED_ATTRIBUTE,
    SL_STATUS_OK);
  attribute_store_register_callback_by_type_and_state_IgnoreArg_callback_function();

  attribute_store_register_callback_by_type_and_state_ExpectAndReturn(
    NULL,
    ATTRIBUTE_GRANTED_SECURITY_KEYS,
    REPORTED_ATTRIBUTE,
    SL_STATUS_OK);
  attribute_store_register_callback_by_type_and_state_IgnoreArg_callback_function();

  attribute_store_register_callback_by_type_and_state_ExpectAndReturn(
    NULL,
    ATTRIBUTE_NETWORK_STATUS,
    REPORTED_ATTRIBUTE,
    SL_STATUS_OK);
  attribute_store_register_callback_by_type_and_state_IgnoreArg_callback_function();

  attribute_store_register_callback_by_type_and_state_ExpectAndReturn(
    NULL,
    ATTRIBUTE_COMMAND_CLASS_WAKE_UP_INTERVAL,
    REPORTED_ATTRIBUTE,
    SL_STATUS_OK);
  attribute_store_register_callback_by_type_and_state_IgnoreArg_callback_function();

  // Initialize now
  process_start(&ucl_node_state_process, NULL);
  contiki_test_helper_run(0);
}

// Stub / intercept functions
static sl_status_t attribute_store_callback_registration_stub(
  attribute_store_node_changed_callback_t callback_function,
  attribute_store_type_t type,
  attribute_store_node_value_state_t value_state,
  int cmock_num_calls)
{
  TEST_ASSERT_EQUAL(REPORTED_ATTRIBUTE, value_state);

  if (type == ATTRIBUTE_NODE_ID) {
    node_id_update_callback_save = callback_function;
  } else if ((type == ATTRIBUTE_GRANTED_SECURITY_KEYS)
             || (type == ATTRIBUTE_COMMAND_CLASS_WAKE_UP_INTERVAL)
             || (type == ATTRIBUTE_NETWORK_STATUS)) {
    network_status_update_callback_save = callback_function;
  } else {
    TEST_ASSERT_TRUE_MESSAGE(
      false,
      "Attribute Store callback registations with unexpected type.");
  }
  return SL_STATUS_OK;
}

static void uic_mqtt_dotdot_state_interview_callback_set_stub(
  const uic_mqtt_dotdot_state_interview_callback_t callback_type,
  int cmock_num_calls)
{
  uic_mqtt_dotdot_state_interview_callback_save = callback_type;
}

static void uic_mqtt_dotdot_state_remove_offline_callback_set_stub(
  const uic_mqtt_dotdot_state_remove_offline_callback_t callback_type,
  int cmock_num_calls)
{
  uic_mqtt_dotdot_state_remove_offline_callback_save = callback_type;
}

static void uic_mqtt_dotdot_state_discover_neighbors_callback_set_stub(
  const uic_mqtt_dotdot_state_discover_neighbors_callback_t callback_type,
  int cmock_num_calls)
{
  uic_mqtt_dotdot_state_discover_neighbors_callback_save = callback_type;
}

// Helper verification functions
static void
  get_node_network_status_verification(attribute_store_node_t test_node,
                                       node_state_topic_state_t return_value)
{
  attribute_store_node_t test_network_status_node = 4;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node,
    ATTRIBUTE_NETWORK_STATUS,
    0,
    test_network_status_node);

  network_status = return_value;
  attribute_store_get_reported_ExpectAndReturn(test_network_status_node,
                                               NULL,
                                               sizeof(node_state_topic_state_t),
                                               SL_STATUS_OK);
  attribute_store_get_reported_IgnoreArg_value();
  attribute_store_get_reported_ReturnMemThruPtr_value(&network_status,
                                                      sizeof(network_status));
}

static void get_node_security_verification(attribute_store_node_t test_node,
                                           zwave_keyset_t return_value)
{
  attribute_store_node_t test_granted_keys_node = 8;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node,
    ATTRIBUTE_GRANTED_SECURITY_KEYS,
    0,
    test_granted_keys_node);

  granted_keys = return_value;
  attribute_store_read_value_ExpectAndReturn(test_granted_keys_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(zwave_keyset_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(&granted_keys,
                                                         sizeof(granted_keys));
}
static void get_get_node_maximum_command_delay_verification(
  attribute_store_node_t test_node, uint32_t maximum_delay)
{
  get_zpc_node_id_node_ExpectAndReturn(test_node + 1);

  test_node_id = 56;
  attribute_store_read_value_ExpectAndReturn(test_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(zwave_node_id_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(&test_node_id,
                                                         sizeof(test_node_id));

  zwave_get_operating_mode_ExpectAndReturn(test_node_id, OPERATING_MODE_NL);

  attribute_store_node_t test_endpoint_node = 45;
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    test_node,
    ATTRIBUTE_ENDPOINT_ID,
    REPORTED_ATTRIBUTE,
    NULL,
    sizeof(zwave_endpoint_id_t),
    0,
    test_endpoint_node);
  attribute_store_get_node_child_by_value_IgnoreArg_value();
  attribute_store_node_t test_wake_up_setting_node = 231;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_WAKE_UP_SETTING,
    0,
    test_wake_up_setting_node);
  attribute_store_node_t test_wake_up_interval_node = 40;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_wake_up_setting_node,
    ATTRIBUTE_COMMAND_CLASS_WAKE_UP_INTERVAL,
    0,
    test_wake_up_interval_node);

  if (maximum_delay != MAX_COMMAND_DELAY_UNKNOWN) {
    delay_value = maximum_delay;
    attribute_store_read_value_ExpectAndReturn(test_wake_up_interval_node,
                                               REPORTED_ATTRIBUTE,
                                               NULL,
                                               sizeof(uint32_t),
                                               SL_STATUS_OK);
    attribute_store_read_value_IgnoreArg_read_value();
    attribute_store_read_value_ReturnMemThruPtr_read_value(&delay_value,
                                                           sizeof(delay_value));
  } else {
    attribute_store_read_value_ExpectAndReturn(test_wake_up_interval_node,
                                               REPORTED_ATTRIBUTE,
                                               NULL,
                                               sizeof(uint32_t),
                                               SL_STATUS_FAIL);
    attribute_store_read_value_IgnoreArg_read_value();
  }
}

static void update_state_node_verification(
  attribute_store_node_t attribute_store_test_node)
{
  attribute_store_is_value_defined_ExpectAndReturn(attribute_store_test_node,
                                                   REPORTED_ATTRIBUTE,
                                                   true);

  get_node_network_status_verification(attribute_store_test_node,
                                       NODE_STATE_TOPIC_INTERVIEWING);
  get_node_security_verification(attribute_store_test_node, 0x00);
  get_get_node_maximum_command_delay_verification(attribute_store_test_node, 0);
  attribute_store_network_helper_get_unid_from_node_ExpectAndReturn(
    attribute_store_test_node,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_unid_from_node_IgnoreArg_unid();
  attribute_store_network_helper_get_unid_from_node_ReturnMemThruPtr_unid(
    test_unid,
    sizeof(unid_t));

  // Publish the state
  uic_mqtt_publish_Expect(expected_state_topic,
                          expected_state_message_1,
                          sizeof(expected_state_message_1) - 1,
                          true);
}

void test_ucl_node_state_init()
{
  TEST_ASSERT_NOT_NULL(node_id_update_callback_save);
  update_state_node_verification(node_id_in_network_1);
  node_id_update_callback_save(node_id_in_network_1, ATTRIBUTE_CREATED);
}

void test_ucl_node_id_attribute_update_undefined()
{
  // This will create the on_pan_node_update
  TEST_ASSERT_NOT_NULL(node_id_update_callback_save);
  attribute_store_node_t test_updated_node = node_id_in_network_2;
  // Nothing should happen if NodeID value is undefined.
  attribute_store_is_value_defined_ExpectAndReturn(test_updated_node,
                                                   REPORTED_ATTRIBUTE,
                                                   false);
  node_id_update_callback_save(test_updated_node, ATTRIBUTE_CREATED);
}

void test_ucl_node_id_attribute_update_happy_case()
{
  TEST_ASSERT_NOT_NULL(node_id_update_callback_save);
  attribute_store_node_t test_updated_node = 58;
  // Try again with a defined value
  attribute_store_is_value_defined_ExpectAndReturn(test_updated_node,
                                                   REPORTED_ATTRIBUTE,
                                                   true);

  attribute_store_network_helper_get_unid_from_node_ExpectAndReturn(
    test_updated_node,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_unid_from_node_IgnoreArg_unid();
  attribute_store_network_helper_get_unid_from_node_ReturnMemThruPtr_unid(
    test_unid,
    sizeof(unid_t));

  get_node_network_status_verification(test_updated_node,
                                       NODE_STATE_TOPIC_STATE_INCLUDED);
  get_node_security_verification(test_updated_node, 0x87);
  get_get_node_maximum_command_delay_verification(test_updated_node,
                                                  MAX_COMMAND_DELAY_UNKNOWN);

  uic_mqtt_publish_Expect(expected_state_topic,
                          expected_state_message_2,
                          sizeof(expected_state_message_2) - 1,
                          true);

  node_id_update_callback_save(test_updated_node, ATTRIBUTE_UPDATED);
}

void test_ucl_node_id_attribute_delete_case()
{
  TEST_ASSERT_NOT_NULL(node_id_update_callback_save);
  attribute_store_node_t test_delete_node = 0x067;
  attribute_store_network_helper_get_unid_from_node_ExpectAndReturn(
    test_delete_node,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_unid_from_node_IgnoreArg_unid();
  attribute_store_network_helper_get_unid_from_node_ReturnMemThruPtr_unid(
    test_unid,
    sizeof(unid_t));
  uic_mqtt_unretain_Expect(expected_state_topic);
  node_id_update_callback_save(test_delete_node, ATTRIBUTE_DELETED);
}

void test_ucl_network_status_attribute_update_happy_case()
{
  TEST_ASSERT_NOT_NULL(network_status_update_callback_save);

  test_ucl_node_state_init();
  attribute_store_node_t test_updated_node = 0xABCD;
  test_node_id_node                        = node_id_in_network_1;
  attribute_store_get_first_parent_with_type_ExpectAndReturn(test_updated_node,
                                                             ATTRIBUTE_NODE_ID,
                                                             test_node_id_node);

  get_node_network_status_verification(test_node_id_node,
                                       NODE_STATE_TOPIC_STATE_INCLUDED);
  get_node_security_verification(test_node_id_node, 0x87);
  get_get_node_maximum_command_delay_verification(test_node_id_node,
                                                  MAX_COMMAND_DELAY_UNKNOWN);
  attribute_store_network_helper_get_unid_from_node_ExpectAndReturn(
    test_node_id_node,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_unid_from_node_IgnoreArg_unid();
  attribute_store_network_helper_get_unid_from_node_ReturnMemThruPtr_unid(
    test_unid,
    sizeof(unid_t));
  // Mock for publish endpoint list
  get_node_network_status_verification(test_node_id_node,
                                       NODE_STATE_TOPIC_STATE_INCLUDED);
  attribute_store_network_helper_get_unid_from_node_ExpectAndReturn(
    test_node_id_node,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_unid_from_node_IgnoreArg_unid();
  attribute_store_network_helper_get_unid_from_node_ReturnMemThruPtr_unid(
    test_unid,
    sizeof(unid_t));
  attribute_store_get_node_child_count_IgnoreAndReturn(0);
  const static char expected_base_topic_endpoint_id_list[]
    = "ucl/by-unid/zw-DEADBEEF-0009";
  uic_mqtt_dotdot_state_endpoint_id_list_publish_ExpectAndReturn(
    expected_base_topic_endpoint_id_list,
    0,
    NULL,
    0xFF,
    SL_STATUS_OK);
  uic_mqtt_publish_Expect(expected_state_topic,
                          expected_state_message_2,
                          sizeof(expected_state_message_2) - 1,
                          true);
  network_status_update_callback_save(test_updated_node, ATTRIBUTE_UPDATED);
}

void test_ucl_node_state_interview_command_zpc_node()
{
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_state_interview_callback_save);
  static dotdot_unid_t unid_test = "zw-DEADBEEF-0001";
  is_zpc_unid_ExpectAndReturn(unid_test, true);
  TEST_ASSERT_EQUAL_UINT32(
    SL_STATUS_NOT_SUPPORTED,
    uic_mqtt_dotdot_state_interview_callback_save(unid_test, 0, 0));
}

void test_ucl_node_state_interview_command_cb_type_check()
{
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_state_interview_callback_save);
  static dotdot_unid_t unid_test = "zw-DEADBEEF-0001";
  is_zpc_unid_ExpectAndReturn(unid_test, false);
  TEST_ASSERT_EQUAL_UINT32(
    SL_STATUS_OK,
    uic_mqtt_dotdot_state_interview_callback_save(unid_test, 0, 1));
}

void test_ucl_node_state_interview_command_intiate_interview()
{
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_state_interview_callback_save);
  static dotdot_unid_t unid_test = "zw-DEADBEEF-0001";
  is_zpc_unid_ExpectAndReturn(unid_test, false);
  ucl_mqtt_initiate_node_interview_ExpectAndReturn(unid_test, SL_STATUS_OK);
  TEST_ASSERT_EQUAL_UINT32(
    SL_STATUS_OK,
    uic_mqtt_dotdot_state_interview_callback_save(unid_test, 0, 0));
}

void test_ucl_node_state_remove_offline_command_zpc_node()
{
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_state_remove_offline_callback_save);
  static dotdot_unid_t unid_test = "zw-DEADBEEF-0001";
  is_zpc_unid_ExpectAndReturn(unid_test, true);
  TEST_ASSERT_EQUAL_UINT32(
    SL_STATUS_NOT_SUPPORTED,
    uic_mqtt_dotdot_state_remove_offline_callback_save(unid_test, 0, 0));
}

void test_ucl_node_state_remove_offline_command_cb_type_check()
{
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_state_remove_offline_callback_save);
  static dotdot_unid_t unid_test = "zw-DEADBEEF-0001";
  is_zpc_unid_ExpectAndReturn(unid_test, false);
  TEST_ASSERT_EQUAL_UINT32(
    SL_STATUS_OK,
    uic_mqtt_dotdot_state_remove_offline_callback_save(unid_test, 0, 1));
}

void test_ucl_node_state_remove_offline_command()
{
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_state_remove_offline_callback_save);
  static dotdot_unid_t unid_test = "zw-DEADBEEF-0001";
  is_zpc_unid_ExpectAndReturn(unid_test, false);
  ucl_network_management_remove_offline_node_ExpectAndReturn(0x01,
                                                             SL_STATUS_OK);
  TEST_ASSERT_EQUAL_UINT32(
    SL_STATUS_OK,
    uic_mqtt_dotdot_state_remove_offline_callback_save(unid_test, 0, 0));
}

void test_ucl_node_state_discover_neighbors_command_zpc_node()
{
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_state_discover_neighbors_callback_save);
  static dotdot_unid_t unid_test = "zw-DEADBEEF-0001";
  is_zpc_unid_ExpectAndReturn(unid_test, true);
  TEST_ASSERT_EQUAL_UINT32(
    SL_STATUS_NOT_SUPPORTED,
    uic_mqtt_dotdot_state_discover_neighbors_callback_save(unid_test, 0, 0));
}

void test_ucl_node_state_discover_neighbors_command_cb_type_check()
{
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_state_discover_neighbors_callback_save);
  static dotdot_unid_t unid_test = "zw-DEADBEEF-0001";
  is_zpc_unid_ExpectAndReturn(unid_test, false);
  TEST_ASSERT_EQUAL_UINT32(
    SL_STATUS_OK,
    uic_mqtt_dotdot_state_discover_neighbors_callback_save(unid_test, 0, 1));
}

void test_ucl_node_state_discover_neighbors_command()
{
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_state_discover_neighbors_callback_save);
  static dotdot_unid_t unid_test = "zw-DEADBEEF-0001";
  is_zpc_unid_ExpectAndReturn(unid_test, false);
  ucl_network_management_request_node_neighbor_update_ExpectAndReturn(
    0x01,
    SL_STATUS_OK);
  TEST_ASSERT_EQUAL_UINT32(
    SL_STATUS_OK,
    uic_mqtt_dotdot_state_discover_neighbors_callback_save(unid_test, 0, 0));
}

void test_teardown()
{
  test_home_id_node = 0x44;
  get_zpc_network_node_ExpectAndReturn(test_home_id_node);
  attribute_store_get_node_child_count_ExpectAndReturn(test_home_id_node, 1);
  attribute_store_get_node_child_by_type_ExpectAndReturn(test_home_id_node,
                                                         ATTRIBUTE_NODE_ID,
                                                         0,
                                                         node_id_in_network_1);
  get_node_security_verification(node_id_in_network_1, 0x80);
  get_get_node_maximum_command_delay_verification(node_id_in_network_1, 350);
  attribute_store_network_helper_get_unid_from_node_ExpectAndReturn(
    node_id_in_network_1,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_unid_from_node_IgnoreArg_unid();
  attribute_store_network_helper_get_unid_from_node_ReturnMemThruPtr_unid(
    test_unid,
    sizeof(unid_t));
  uic_mqtt_publish_Expect(expected_state_topic,
                          expected_state_message_3,
                          sizeof(expected_state_message_3) - 1,
                          true);

  uic_mqtt_unretain_by_regex_Expect(expected_state_topic);
  uic_mqtt_unretain_by_regex_IgnoreArg_regex();

  // Kick the process exit
  process_exit(&ucl_node_state_process);
  contiki_test_helper_run(0);
}

void test_ucl_publish_endpoint_list()
{
  TEST_ASSERT_NOT_NULL(network_status_update_callback_save);

  test_ucl_node_state_init();
  attribute_store_node_t test_updated_node = 0xABCD;
  test_node_id_node                        = node_id_in_network_1;
  attribute_store_get_first_parent_with_type_ExpectAndReturn(test_updated_node,
                                                             ATTRIBUTE_NODE_ID,
                                                             test_node_id_node);

  get_node_network_status_verification(test_node_id_node,
                                       NODE_STATE_TOPIC_STATE_INCLUDED);
  get_node_security_verification(test_node_id_node, 0x87);
  get_get_node_maximum_command_delay_verification(test_node_id_node,
                                                  MAX_COMMAND_DELAY_UNKNOWN);
  attribute_store_network_helper_get_unid_from_node_ExpectAndReturn(
    test_node_id_node,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_unid_from_node_IgnoreArg_unid();
  attribute_store_network_helper_get_unid_from_node_ReturnMemThruPtr_unid(
    test_unid,
    sizeof(unid_t));
  // Mock for publish endpoint list
  get_node_network_status_verification(test_node_id_node,
                                       NODE_STATE_TOPIC_STATE_INCLUDED);
  attribute_store_network_helper_get_unid_from_node_ExpectAndReturn(
    test_node_id_node,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_unid_from_node_IgnoreArg_unid();
  attribute_store_network_helper_get_unid_from_node_ReturnMemThruPtr_unid(
    test_unid,
    sizeof(unid_t));
  attribute_store_get_node_child_count_IgnoreAndReturn(1);
  attribute_store_node_t test_ep_node = 0xAFCD;
  attribute_store_get_node_child_ExpectAndReturn(test_node_id_node,
                                                 0,
                                                 test_ep_node);
  attribute_store_get_node_type_ExpectAndReturn(test_ep_node,
                                                ATTRIBUTE_ENDPOINT_ID);
  attribute_store_get_node_child_ExpectAndReturn(test_node_id_node,
                                                 0,
                                                 test_ep_node);
  attribute_store_is_value_defined_ExpectAndReturn(test_ep_node,
                                                   REPORTED_ATTRIBUTE,
                                                   1);
  attribute_store_get_node_attribute_value_ExpectAndReturn(test_ep_node,
                                                           REPORTED_ATTRIBUTE,
                                                           NULL,
                                                           NULL,
                                                           SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  static zwave_endpoint_id_t test_endpoint = 3;
  static uint8_t test_endpoint_size        = sizeof(zwave_endpoint_id_t);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value(&test_endpoint);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &test_endpoint_size);
  const static char expected_base_topic_endpoint_id_list[]
    = "ucl/by-unid/zw-DEADBEEF-0009";
  uic_mqtt_dotdot_state_endpoint_id_list_publish_ExpectAndReturn(
    expected_base_topic_endpoint_id_list,
    test_endpoint_size,
    &test_endpoint,
    0xFF,
    SL_STATUS_OK);
  uic_mqtt_publish_Expect(expected_state_topic,
                          expected_state_message_2,
                          sizeof(expected_state_message_2) - 1,
                          true);
  network_status_update_callback_save(test_updated_node, ATTRIBUTE_UPDATED);
}