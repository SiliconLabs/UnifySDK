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
#include "zpc_attribute_resolver_fixt.h"
#include "zpc_attribute_resolver.h"

// Test includes
#include "zpc_attribute_resolver_test.hpp"
#include "unity.h"

// UIC Mock includes
#include "attribute_store_mock.h"
#include "attribute_store_helper_mock.h"
#include "attribute_resolver_mock.h"
#include "attribute_resolver_rule_mock.h"

// ZPC Mock includes
#include "zpc_attribute_store_network_helper_mock.h"
#include "zwave_tx_mock.h"
#include "zwave_tx_groups_mock.h"
#include "zwave_command_class_supervision_mock.h"
#include "zwave_tx_scheme_selector_mock.h"
#include "zwave_controller_utils_mock.h"
#include "zwave_controller_keyset_mock.h"
#include "zwave_utils_mock.h"

// Interface includes
#include "sl_status.h"
#include "ZW_classcmd.h"

// Generic includes
#include <string.h>

// Static test variables
static resolver_on_set_rule_registered_t set_rule_notification_function = NULL;
static attribute_store_node_changed_callback_t set_notification_function = NULL;
static attribute_resolver_config_t received_resolver_config             = {};
static on_zwave_tx_send_data_complete_t received_send_data_complete     = NULL;
static uint8_t custom_handler_calls                                     = 0;
static void *received_user                                              = NULL;

// Stub functions.
// Saving registration/init data to the UIC Attribute resolver.
static void
  register_set_rule_listener_Stub(resolver_on_set_rule_registered_t function,
                                  int cmock_num_calls)
{
  set_rule_notification_function = function;
}

static void custom_send_event_handler(attribute_store_node_t node,
                                      resolver_rule_type_t rule_type,
                                      zpc_resolver_event_t event)
{
  custom_handler_calls++;
}

static sl_status_t
  attribute_resolver_init_stub(attribute_resolver_config_t resolver_config,
                               int cmock_num_calls)
{
  received_resolver_config = resolver_config;
  return SL_STATUS_OK;
}

static sl_status_t attribute_store_register_callback_stub(
  attribute_store_node_changed_callback_t callback_function,
  attribute_store_type_t type,
  attribute_store_node_value_state_t value_state,
  int cmock_num_calls)
{
  set_notification_function = callback_function;
  return SL_STATUS_OK;
}

attribute_resolver_function_t test;

static sl_status_t rule_function_stub(attribute_store_node_t node,
                                      uint8_t *frame,
                                      uint16_t *frame_len)
{
  memcpy(frame, test_frame_data, sizeof(test_frame_data));
  *frame_len = sizeof(test_frame_data);
  return SL_STATUS_OK;
}

static sl_status_t zwave_tx_send_data_stub(
  const zwave_controller_connection_info_t *connection,
  uint16_t data_length,
  const uint8_t *data,
  const zwave_tx_options_t *tx_options,
  const on_zwave_tx_send_data_complete_t on_send_complete,
  void *user,
  zwave_tx_session_id_t *session,
  int cmock_num_calls)
{
  received_user               = user;
  received_send_data_complete = on_send_complete;

  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  set_rule_notification_function             = NULL;
  set_notification_function                  = NULL;
  received_resolver_config.send_init         = NULL;
  received_resolver_config.send              = NULL;
  received_resolver_config.get_retry_timeout = 0;
  received_resolver_config.get_retry_count   = 0;
  received_send_data_complete                = NULL;
  received_user                              = NULL;
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  received_send_data_complete = NULL;
  received_user               = NULL;
  custom_handler_calls        = 0;
}

void test_zpc_attribute_resolver_init()
{
  attribute_resolver_register_set_rule_listener_Stub(
    register_set_rule_listener_Stub);

  attribute_resolver_init_Stub(&attribute_resolver_init_stub);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, zpc_attribute_resolver_init());

  attribute_store_type_t test_type = 348573;
  attribute_store_register_callback_by_type_and_state_Stub(
    attribute_store_register_callback_stub);
  attribute_store_register_callback_by_type_and_state_ExpectAndReturn(
    NULL,
    test_type,
    DESIRED_ATTRIBUTE,
    SL_STATUS_OK);

  TEST_ASSERT_NOT_NULL(set_rule_notification_function);
  set_rule_notification_function(test_type);
}

void test_zpc_attribute_resolver_send_get_happy_case()
{
  attribute_store_node_t test_node = 3;
  attribute_store_type_t test_type = 0x25;
  const uint8_t frame_data[]       = {0x01, 0x01, 0xFF, 0x00, 0x0A, 0x04};
  bool is_set                      = false;

  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    test_node,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_node_id(
    &zwave_node_id_1);
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &zwave_endpoint_id_1);

  zwave_tx_scheme_get_node_connection_info_Expect(zwave_node_id_1,
                                                  zwave_endpoint_id_1,
                                                  NULL);
  zwave_tx_scheme_get_node_connection_info_IgnoreArg_connection_info();
  zwave_tx_scheme_get_node_connection_info_ReturnThruPtr_connection_info(
    &connection_info_1);

  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_NODE_INTERVIEW_PRIORITY,
    1,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();
  zwave_tx_scheme_get_node_tx_options_ReturnThruPtr_tx_options(&tx_options_1);

  zwave_tx_send_data_AddCallback(&zwave_tx_send_data_stub);
  zwave_tx_send_data_ExpectWithArrayAndReturn(&connection_info_1,
                                              sizeof(connection_info_1),
                                              sizeof(frame_data),
                                              frame_data,
                                              sizeof(frame_data),
                                              &tx_options_1,
                                              sizeof(tx_options_1),
                                              NULL,
                                              NULL,
                                              0,
                                              NULL,
                                              0,
                                              SL_STATUS_OK);
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_session();

  TEST_ASSERT_NOT_NULL(received_resolver_config.send);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    received_resolver_config.send(test_node,
                                                  frame_data,
                                                  sizeof(frame_data),
                                                  is_set));

  // Calling it a second time will not have any effect: (already in resolution list)
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    received_resolver_config.send(test_node,
                                                  frame_data,
                                                  sizeof(frame_data),
                                                  is_set));

  // Sending has been triggered. Now the callback.
  attribute_store_get_node_type_ExpectAndReturn(test_node, test_type);
  on_resolver_send_data_complete_Expect(RESOLVER_SEND_STATUS_FAIL,
                                        0,
                                        test_node,
                                        RESOLVER_GET_RULE);
  TEST_ASSERT_NOT_NULL(received_send_data_complete);
  received_send_data_complete(TRANSMIT_COMPLETE_FAIL, NULL, received_user);
}

void test_zpc_attribute_resolver_send_error_cant_find_zwave_ids()
{
  attribute_store_node_t test_node = 2;

  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    test_node,
    NULL,
    NULL,
    SL_STATUS_WOULD_OVERFLOW);
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();

  TEST_ASSERT_NOT_NULL(received_resolver_config.send);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    received_resolver_config.send(test_node, NULL, 0, true));
}

void test_zpc_attribute_resolver_send_send_status_fail_no_supervision()
{
  attribute_store_node_t test_node = 3;
  attribute_store_type_t test_type = 0x25;
  const uint8_t frame_data[]       = {0x9F, 0x22, 0xFF, 0x00, 0x0A, 0x04};
  bool is_set                      = false;

  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    test_node,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_node_id(
    &zwave_node_id_2);
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &zwave_endpoint_id_2);

  zwave_tx_scheme_get_node_connection_info_Expect(zwave_node_id_2,
                                                  zwave_endpoint_id_2,
                                                  NULL);
  zwave_tx_scheme_get_node_connection_info_IgnoreArg_connection_info();
  zwave_tx_scheme_get_node_connection_info_ReturnThruPtr_connection_info(
    &connection_info_2);

  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_NODE_INTERVIEW_PRIORITY,
    1,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();
  zwave_tx_scheme_get_node_tx_options_ReturnThruPtr_tx_options(&tx_options_2);

  zwave_tx_send_data_AddCallback(NULL);
  zwave_tx_send_data_ExpectWithArrayAndReturn(&connection_info_2,
                                              sizeof(connection_info_2),
                                              sizeof(frame_data),
                                              frame_data,
                                              sizeof(frame_data),
                                              &tx_options_2,
                                              sizeof(tx_options_2),
                                              NULL,
                                              NULL,
                                              0,
                                              NULL,
                                              0,
                                              SL_STATUS_FAIL);
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_session();

  // tx send status fail will trigger an immediate callback:
  attribute_store_get_node_type_ExpectAndReturn(test_node, test_type);
  on_resolver_send_data_complete_Expect(RESOLVER_SEND_STATUS_FAIL,
                                        0,
                                        test_node,
                                        RESOLVER_GET_RULE);

  TEST_ASSERT_NOT_NULL(received_resolver_config.send);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    received_resolver_config.send(test_node,
                                                  frame_data,
                                                  sizeof(frame_data),
                                                  is_set));
}

void test_zpc_attribute_resolver_send_send_status_fail_with_supervision()
{
  attribute_store_node_t test_node = 3;
  attribute_store_type_t test_type = 0x25;
  const uint8_t frame_data[]       = {0x9F, 0x22, 0xFF, 0x00, 0x0A, 0x04};
  bool is_set                      = true;

  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    test_node,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_node_id(
    &zwave_node_id_2);
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &zwave_endpoint_id_2);

  zwave_tx_scheme_get_node_connection_info_Expect(zwave_node_id_2,
                                                  zwave_endpoint_id_2,
                                                  NULL);
  zwave_tx_scheme_get_node_connection_info_IgnoreArg_connection_info();
  zwave_tx_scheme_get_node_connection_info_ReturnThruPtr_connection_info(
    &connection_info_2);

  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_NODE_INTERVIEW_PRIORITY,
    0,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();
  zwave_tx_scheme_get_node_tx_options_ReturnThruPtr_tx_options(&tx_options_2);

  zwave_node_supports_command_class_ExpectAndReturn(0x6C,
                                                    zwave_node_id_2,
                                                    zwave_endpoint_id_2,
                                                    true);

  zwave_command_class_supervision_send_data_AddCallback(NULL);
  zwave_command_class_supervision_send_data_ExpectWithArrayAndReturn(
    &connection_info_2,
    sizeof(connection_info_2),
    sizeof(frame_data),
    frame_data,
    sizeof(frame_data),
    &tx_options_2,
    sizeof(tx_options_2),
    NULL,
    NULL,
    0,
    NULL,
    0,
    SL_STATUS_FAIL);
  zwave_command_class_supervision_send_data_IgnoreArg_on_supervision_complete();
  zwave_command_class_supervision_send_data_IgnoreArg_user();
  zwave_command_class_supervision_send_data_IgnoreArg_session();

  // tx send status fail will trigger an immediate callback:
  attribute_store_get_node_type_ExpectAndReturn(test_node, test_type);
  on_resolver_send_data_complete_Expect(
    RESOLVER_SEND_STATUS_OK_EXECUTION_FAILED,
    0,
    test_node,
    RESOLVER_SET_RULE);

  TEST_ASSERT_NOT_NULL(received_resolver_config.send);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    received_resolver_config.send(test_node,
                                                  frame_data,
                                                  sizeof(frame_data),
                                                  is_set));
}

void test_zpc_attribute_resolver_send_set_no_supervision_happy_case()
{
  attribute_store_node_t test_node = 75;
  attribute_store_type_t test_type = 930489;
  const uint8_t frame_data[]       = {0x01, 0x01, 0xFF, 0x00, 0x0A, 0x04};
  bool is_set                      = true;

  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    test_node,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_node_id(
    &zwave_node_id_1);
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &zwave_endpoint_id_1);

  zwave_tx_scheme_get_node_connection_info_Expect(zwave_node_id_1,
                                                  zwave_endpoint_id_1,
                                                  NULL);
  zwave_tx_scheme_get_node_connection_info_IgnoreArg_connection_info();
  zwave_tx_scheme_get_node_connection_info_ReturnThruPtr_connection_info(
    &connection_info_1);

  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_NODE_INTERVIEW_PRIORITY,
    0,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();
  zwave_tx_scheme_get_node_tx_options_ReturnThruPtr_tx_options(&tx_options_1);

  zwave_node_supports_command_class_ExpectAndReturn(0x6C,
                                                    zwave_node_id_1,
                                                    zwave_endpoint_id_1,
                                                    false);

  zwave_tx_send_data_AddCallback(&zwave_tx_send_data_stub);
  zwave_tx_send_data_ExpectWithArrayAndReturn(&connection_info_1,
                                              sizeof(connection_info_1),
                                              sizeof(frame_data),
                                              frame_data,
                                              sizeof(frame_data),
                                              &tx_options_1,
                                              sizeof(tx_options_1),
                                              NULL,
                                              NULL,
                                              0,
                                              NULL,
                                              0,
                                              SL_STATUS_OK);
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_session();

  TEST_ASSERT_NOT_NULL(received_resolver_config.send);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    received_resolver_config.send(test_node,
                                                  frame_data,
                                                  sizeof(frame_data),
                                                  is_set));

  // Sending has been triggered. Now the callback.
  attribute_store_get_node_type_ExpectAndReturn(test_node, test_type);
  on_resolver_send_data_complete_Expect(RESOLVER_SEND_STATUS_OK,
                                        0,
                                        test_node,
                                        RESOLVER_SET_RULE);
  TEST_ASSERT_NOT_NULL(received_send_data_complete);
  received_send_data_complete(TRANSMIT_COMPLETE_OK, NULL, received_user);
}

void test_zpc_attribute_resolver_send_set_supervision_working_happy_case()
{
  attribute_store_node_t test_node = 28647;
  attribute_store_type_t test_type = 382;
  TEST_ASSERT_NOT_NULL(set_notification_function);
  // Notify of incoming set resolution, but only 1 node will just be ignored.
  attribute_store_is_value_defined_ExpectAndReturn(test_node,
                                                   DESIRED_ATTRIBUTE,
                                                   true);
  attribute_store_is_value_matched_ExpectAndReturn(test_node, false);
  set_notification_function(test_node, ATTRIBUTE_UPDATED);

  const uint8_t frame_data[] = {0x01, 0x01, 0xFF, 0x00, 0x0A, 0x04};
  bool is_set                = true;

  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    test_node,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_node_id(
    &zwave_node_id_1);
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &zwave_endpoint_id_1);

  zwave_tx_scheme_get_node_connection_info_Expect(zwave_node_id_1,
                                                  zwave_endpoint_id_1,
                                                  NULL);
  zwave_tx_scheme_get_node_connection_info_IgnoreArg_connection_info();
  zwave_tx_scheme_get_node_connection_info_ReturnThruPtr_connection_info(
    &connection_info_1);

  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_NODE_INTERVIEW_PRIORITY,
    0,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();
  zwave_tx_scheme_get_node_tx_options_ReturnThruPtr_tx_options(&tx_options_1);

  zwave_node_supports_command_class_ExpectAndReturn(0x6C,
                                                    zwave_node_id_1,
                                                    zwave_endpoint_id_1,
                                                    true);

  zwave_command_class_supervision_send_data_AddCallback(
    &zwave_tx_send_data_stub);
  zwave_command_class_supervision_send_data_ExpectWithArrayAndReturn(
    &connection_info_1,
    sizeof(connection_info_1),
    sizeof(frame_data),
    frame_data,
    sizeof(frame_data),
    &tx_options_1,
    sizeof(tx_options_1),
    NULL,
    NULL,
    0,
    NULL,
    0,
    SL_STATUS_OK);
  zwave_command_class_supervision_send_data_IgnoreArg_on_supervision_complete();
  zwave_command_class_supervision_send_data_IgnoreArg_user();
  zwave_command_class_supervision_send_data_IgnoreArg_session();

  TEST_ASSERT_NOT_NULL(received_resolver_config.send);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    received_resolver_config.send(test_node,
                                                  frame_data,
                                                  sizeof(frame_data),
                                                  is_set));

  // Sending has been triggered. Now the callback.
  attribute_store_get_node_type_ExpectAndReturn(test_node, test_type);
  on_resolver_send_data_complete_Expect(
    RESOLVER_SEND_STATUS_OK_EXECUTION_PENDING,
    0,
    test_node,
    RESOLVER_SET_RULE);
  TEST_ASSERT_NOT_NULL(received_send_data_complete);
  received_send_data_complete(SUPERVISION_REPORT_WORKING, NULL, received_user);

  // trying to resolve again will just be ignored
  // as long as the node is making a transition  (supervision working)
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    received_resolver_config.send(test_node,
                                                  frame_data,
                                                  sizeof(frame_data),
                                                  is_set));

  // Node finally reaches the final state.
  attribute_store_get_node_type_ExpectAndReturn(test_node, test_type);
  on_resolver_send_data_complete_Expect(
    RESOLVER_SEND_STATUS_OK_EXECUTION_VERIFIED,
    0,
    test_node,
    RESOLVER_SET_RULE);
  received_send_data_complete(SUPERVISION_REPORT_SUCCESS, NULL, received_user);
}

// Now touching the group logic.
void test_zpc_attribute_resolver_group_2_nodes_happy_case()
{
  attribute_store_node_t test_node_1      = 28647;
  attribute_store_node_t test_node_1_type = 4;
  attribute_store_node_t test_node_2      = 345;
  attribute_store_node_t test_node_2_type = 2;
  TEST_ASSERT_NOT_NULL(set_notification_function);
  attribute_store_is_value_defined_ExpectAndReturn(test_node_1,
                                                   DESIRED_ATTRIBUTE,
                                                   true);
  attribute_store_is_value_matched_ExpectAndReturn(test_node_1, false);
  set_notification_function(test_node_1, ATTRIBUTE_UPDATED);
  attribute_store_is_value_defined_ExpectAndReturn(test_node_2,
                                                   DESIRED_ATTRIBUTE,
                                                   true);
  attribute_store_is_value_matched_ExpectAndReturn(test_node_2, false);
  set_notification_function(test_node_2, ATTRIBUTE_CREATED);

  const uint8_t frame_data[] = {0x01, 0x01, 0xFF, 0x00, 0x0A, 0x04};
  bool is_set                = true;

  zwave_tx_is_group_locked_IgnoreAndReturn(false);

  // assigning test_node_2 in a multicast pool
  is_node_or_parent_paused_ExpectAndReturn(test_node_2, false);
  attribute_store_is_value_defined_ExpectAndReturn(test_node_2,
                                                   REPORTED_ATTRIBUTE,
                                                   true);
  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    test_node_2,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_node_id(
    &zwave_node_id_2);
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &zwave_endpoint_id_1);

  zwave_get_inclusion_protocol_ExpectAndReturn(zwave_node_id_2, protocol_1);
  zwave_tx_scheme_get_node_highest_security_class_ExpectAndReturn(
    zwave_node_id_2,
    encapsulation_1);

  attribute_store_get_node_type_ExpectAndReturn(test_node_2, test_node_2_type);
  attribute_resolver_set_function_ExpectAndReturn(test_node_2_type,
                                                  &rule_function_stub);
  zwave_node_supports_command_class_ExpectAndReturn(0x6C,
                                                    zwave_node_id_2,
                                                    zwave_endpoint_id_1,
                                                    true);

  // assigning test_node_1 in a multicast pool
  is_node_or_parent_paused_ExpectAndReturn(test_node_1, false);
  attribute_store_is_value_defined_ExpectAndReturn(test_node_1,
                                                   REPORTED_ATTRIBUTE,
                                                   true);
  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    test_node_1,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_node_id(
    &zwave_node_id_1);
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &zwave_endpoint_id_1);

  zwave_get_inclusion_protocol_ExpectAndReturn(zwave_node_id_1, protocol_1);
  zwave_tx_scheme_get_node_highest_security_class_ExpectAndReturn(
    zwave_node_id_1,
    encapsulation_1);

  attribute_store_get_node_type_ExpectAndReturn(test_node_1, test_node_1_type);
  attribute_resolver_set_function_ExpectAndReturn(test_node_1_type,
                                                  &rule_function_stub);
  zwave_node_supports_command_class_ExpectAndReturn(0x6C,
                                                    zwave_node_id_1,
                                                    zwave_endpoint_id_1,
                                                    true);

  attribute_store_network_helper_get_node_id_from_node_ExpectAndReturn(
    test_node_2,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_node_id_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_node_id_from_node_ReturnThruPtr_zwave_node_id(
    &zwave_node_id_2);

  // Creating the node list for the Multicast TX Call
  zwave_nodemask_t node_mask = {};
  ZW_ADD_NODE_TO_MASK(zwave_node_id_1, node_mask);
  ZW_ADD_NODE_TO_MASK(zwave_node_id_2, node_mask);

  attribute_store_network_helper_get_node_id_from_node_ExpectAndReturn(
    test_node_2,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_node_id_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_node_id_from_node_ReturnThruPtr_zwave_node_id(
    &zwave_node_id_2);
  attribute_store_network_helper_get_node_id_from_node_ExpectAndReturn(
    test_node_1,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_node_id_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_node_id_from_node_ReturnThruPtr_zwave_node_id(
    &zwave_node_id_1);

  zwave_tx_assign_group_ExpectWithArrayAndReturn(node_mask,
                                                 sizeof(node_mask),
                                                 NULL,
                                                 0,
                                                 SL_STATUS_OK);
  zwave_tx_assign_group_IgnoreArg_group_id();
  zwave_tx_assign_group_ReturnThruPtr_group_id(&group_id_1);

  zwave_tx_scheme_get_node_tx_options_Expect(0xFFFF + 5 * 10, 0, 0, NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();
  zwave_tx_scheme_get_node_tx_options_ReturnThruPtr_tx_options(&tx_options_1);

  zwave_command_class_supervision_send_data_AddCallback(
    &zwave_tx_send_data_stub);
  zwave_command_class_supervision_send_data_ExpectWithArrayAndReturn(
    NULL,
    0,
    sizeof(frame_data),
    frame_data,
    sizeof(frame_data),
    &tx_options_1,
    sizeof(tx_options_1),
    NULL,
    NULL,
    0,
    NULL,
    0,
    SL_STATUS_OK);
  zwave_command_class_supervision_send_data_IgnoreArg_connection();
  zwave_command_class_supervision_send_data_IgnoreArg_on_supervision_complete();
  zwave_command_class_supervision_send_data_IgnoreArg_user();
  zwave_command_class_supervision_send_data_IgnoreArg_session();

  zwave_tx_lock_group_ExpectAndReturn(group_id_1, SL_STATUS_OK);

  // Enqueuing follow-ups here.
  // Node 2 first
  attribute_store_network_helper_get_node_id_from_node_ExpectAndReturn(
    test_node_2,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_node_id_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_node_id_from_node_ReturnThruPtr_zwave_node_id(
    &zwave_node_id_2);
  zwave_get_operating_mode_ExpectAndReturn(zwave_node_id_2, OPERATING_MODE_FL);
  zwave_command_class_supervision_send_data_AddCallback(
    &zwave_tx_send_data_stub);
  zwave_command_class_supervision_send_data_ExpectAndReturn(
    NULL,
    sizeof(test_frame_data),
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    SL_STATUS_OK);
  zwave_command_class_supervision_send_data_IgnoreArg_on_supervision_complete();
  zwave_command_class_supervision_send_data_IgnoreArg_tx_options();
  zwave_command_class_supervision_send_data_IgnoreArg_connection();
  zwave_command_class_supervision_send_data_IgnoreArg_data();
  zwave_command_class_supervision_send_data_IgnoreArg_session();
  zwave_command_class_supervision_send_data_IgnoreArg_user();
  zwave_command_class_supervision_send_data_IgnoreArg_session();

  // Node 1 second
  attribute_store_network_helper_get_node_id_from_node_ExpectAndReturn(
    test_node_1,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_node_id_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_node_id_from_node_ReturnThruPtr_zwave_node_id(
    &zwave_node_id_1);
  zwave_get_operating_mode_ExpectAndReturn(zwave_node_id_1, OPERATING_MODE_AL);

  zwave_command_class_supervision_send_data_ExpectAndReturn(
    NULL,
    sizeof(test_frame_data),
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    SL_STATUS_OK);
  zwave_command_class_supervision_send_data_IgnoreArg_on_supervision_complete();
  zwave_command_class_supervision_send_data_IgnoreArg_tx_options();
  zwave_command_class_supervision_send_data_IgnoreArg_connection();
  zwave_command_class_supervision_send_data_IgnoreArg_data();
  zwave_command_class_supervision_send_data_IgnoreArg_session();
  zwave_command_class_supervision_send_data_IgnoreArg_user();
  zwave_command_class_supervision_send_data_IgnoreArg_session();

  // Finally trigger the send call
  TEST_ASSERT_NOT_NULL(received_resolver_config.send);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    received_resolver_config.send(test_node_1,
                                                  frame_data,
                                                  sizeof(frame_data),
                                                  is_set));

  // trying to resolve again will just be ignored
  // because multicast took care of it
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    received_resolver_config.send(test_node_1,
                                                  frame_data,
                                                  sizeof(frame_data),
                                                  is_set));

  // Our callback system has only caught the 2nd follow-up pointer as last
  // received user pointer, so the follow-up for test_node_1

  // Node finally reaches the final state.
  attribute_store_get_node_type_ExpectAndReturn(test_node_1, test_node_1_type);
  TEST_ASSERT_NOT_NULL(received_send_data_complete);
  on_resolver_send_data_complete_Expect(
    RESOLVER_SEND_STATUS_OK_EXECUTION_VERIFIED,
    0,
    test_node_1,
    RESOLVER_SET_RULE);
  received_send_data_complete(SUPERVISION_REPORT_SUCCESS, NULL, received_user);

  // Reset the pending resolution of test_node_2
  received_resolver_config.send_init();
}

// Now touching the group logic.
void test_zpc_attribute_resolver_group_unknown_protocol()
{
  attribute_store_node_t test_node_1      = 28647;
  attribute_store_node_t test_node_1_type = 56;
  attribute_store_node_t test_node_2      = 345;
  attribute_store_node_t test_node_2_type = 9945;
  TEST_ASSERT_NOT_NULL(set_notification_function);
  attribute_store_is_value_defined_ExpectAndReturn(test_node_1,
                                                   DESIRED_ATTRIBUTE,
                                                   true);
  attribute_store_is_value_matched_ExpectAndReturn(test_node_1, false);
  set_notification_function(test_node_1, ATTRIBUTE_UPDATED);
  attribute_store_is_value_defined_ExpectAndReturn(test_node_2,
                                                   DESIRED_ATTRIBUTE,
                                                   true);
  attribute_store_is_value_matched_ExpectAndReturn(test_node_2, false);
  set_notification_function(test_node_2, ATTRIBUTE_UPDATED);

  const uint8_t frame_data[] = {0x01, 0x01, 0xFF, 0x00, 0x0A, 0x04};
  bool is_set                = true;

  zwave_tx_is_group_locked_IgnoreAndReturn(false);

  // assigning test_node_2 and in a multicast pool
  is_node_or_parent_paused_ExpectAndReturn(test_node_2, true);

  // assigning test_node_1 in a multicast pool
  is_node_or_parent_paused_ExpectAndReturn(test_node_1, false);
  attribute_store_is_value_defined_ExpectAndReturn(test_node_1,
                                                   REPORTED_ATTRIBUTE,
                                                   true);
  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    test_node_1,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_node_id(
    &zwave_node_id_1);
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &zwave_endpoint_id_1);

  zwave_get_inclusion_protocol_ExpectAndReturn(zwave_node_id_1,
                                               protocol_unknown);
  zwave_tx_scheme_get_node_highest_security_class_ExpectAndReturn(
    zwave_node_id_1,
    encapsulation_1);

  attribute_store_get_node_type_ExpectAndReturn(test_node_1, test_node_1_type);
  attribute_resolver_set_function_ExpectAndReturn(test_node_1_type,
                                                  &rule_function_stub);
  zwave_node_supports_command_class_ExpectAndReturn(0x6C,
                                                    zwave_node_id_1,
                                                    zwave_endpoint_id_1,
                                                    true);

  // Fallback on a regular test_node_2 transmission for a set command
  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    test_node_2,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_node_id(
    &zwave_node_id_1);
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &zwave_endpoint_id_1);

  zwave_tx_scheme_get_node_connection_info_Expect(zwave_node_id_1,
                                                  zwave_endpoint_id_1,
                                                  NULL);
  zwave_tx_scheme_get_node_connection_info_IgnoreArg_connection_info();
  zwave_tx_scheme_get_node_connection_info_ReturnThruPtr_connection_info(
    &connection_info_1);

  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_NODE_INTERVIEW_PRIORITY,
    0,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();
  zwave_tx_scheme_get_node_tx_options_ReturnThruPtr_tx_options(&tx_options_1);

  zwave_node_supports_command_class_ExpectAndReturn(0x6C,
                                                    zwave_node_id_1,
                                                    zwave_endpoint_id_1,
                                                    false);

  zwave_tx_send_data_AddCallback(&zwave_tx_send_data_stub);
  zwave_tx_send_data_ExpectWithArrayAndReturn(&connection_info_1,
                                              sizeof(connection_info_1),
                                              sizeof(frame_data),
                                              frame_data,
                                              sizeof(frame_data),
                                              &tx_options_1,
                                              sizeof(tx_options_1),
                                              NULL,
                                              NULL,
                                              0,
                                              NULL,
                                              0,
                                              SL_STATUS_OK);
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_session();

  TEST_ASSERT_NOT_NULL(received_resolver_config.send);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    received_resolver_config.send(test_node_2,
                                                  frame_data,
                                                  sizeof(frame_data),
                                                  is_set));

  // Sending has been triggered. Now the callback.
  attribute_store_get_node_type_ExpectAndReturn(test_node_2, test_node_2_type);
  on_resolver_send_data_complete_Expect(RESOLVER_SEND_STATUS_OK,
                                        0,
                                        test_node_2,
                                        RESOLVER_SET_RULE);
  TEST_ASSERT_NOT_NULL(received_send_data_complete);
  received_send_data_complete(TRANSMIT_COMPLETE_OK, NULL, received_user);

  // Reset the pending resolution of test_node_2
  received_resolver_config.send_init();
}

void test_zpc_attribute_resolver_group_no_reported_value()
{
  attribute_store_node_t test_node_1      = 28647;
  attribute_store_node_t test_node_1_type = 6;
  attribute_store_node_t test_node_2      = 345;
  attribute_store_node_t test_node_2_type = 9945;

  TEST_ASSERT_NOT_NULL(set_notification_function);
  attribute_store_is_value_defined_ExpectAndReturn(test_node_1,
                                                   DESIRED_ATTRIBUTE,
                                                   true);
  attribute_store_is_value_matched_ExpectAndReturn(test_node_1, false);
  set_notification_function(test_node_1, ATTRIBUTE_UPDATED);
  attribute_store_is_value_defined_ExpectAndReturn(test_node_2,
                                                   DESIRED_ATTRIBUTE,
                                                   true);
  attribute_store_is_value_matched_ExpectAndReturn(test_node_2, false);
  set_notification_function(test_node_2, ATTRIBUTE_UPDATED);

  const uint8_t frame_data[] = {0x01, 0x01, 0xFF, 0x00, 0x0A, 0x04};
  bool is_set                = true;

  zwave_tx_is_group_locked_IgnoreAndReturn(false);

  // assigning test_node_2 and in a multicast pool
  is_node_or_parent_paused_ExpectAndReturn(test_node_2, false);
  attribute_store_is_value_defined_ExpectAndReturn(test_node_2,
                                                   REPORTED_ATTRIBUTE,
                                                   false);
  // assigning test_node_1 in a multicast pool
  is_node_or_parent_paused_ExpectAndReturn(test_node_1, false);
  attribute_store_is_value_defined_ExpectAndReturn(test_node_1,
                                                   REPORTED_ATTRIBUTE,
                                                   true);
  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    test_node_1,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_node_id(
    &zwave_node_id_1);
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &zwave_endpoint_id_1);

  zwave_get_inclusion_protocol_ExpectAndReturn(zwave_node_id_1,
                                               protocol_unknown);
  zwave_tx_scheme_get_node_highest_security_class_ExpectAndReturn(
    zwave_node_id_1,
    encapsulation_1);

  attribute_store_get_node_type_ExpectAndReturn(test_node_1, test_node_1_type);
  attribute_resolver_set_function_ExpectAndReturn(test_node_1_type,
                                                  &rule_function_stub);
  zwave_node_supports_command_class_ExpectAndReturn(0x6C,
                                                    zwave_node_id_1,
                                                    zwave_endpoint_id_1,
                                                    true);

  // Fallback on a regular test_node_2 transmission for a set command
  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    test_node_2,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_node_id(
    &zwave_node_id_1);
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &zwave_endpoint_id_1);

  zwave_tx_scheme_get_node_connection_info_Expect(zwave_node_id_1,
                                                  zwave_endpoint_id_1,
                                                  NULL);
  zwave_tx_scheme_get_node_connection_info_IgnoreArg_connection_info();
  zwave_tx_scheme_get_node_connection_info_ReturnThruPtr_connection_info(
    &connection_info_1);

  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_NODE_INTERVIEW_PRIORITY,
    0,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();
  zwave_tx_scheme_get_node_tx_options_ReturnThruPtr_tx_options(&tx_options_1);

  zwave_node_supports_command_class_ExpectAndReturn(0x6C,
                                                    zwave_node_id_1,
                                                    zwave_endpoint_id_1,
                                                    false);

  zwave_tx_send_data_AddCallback(&zwave_tx_send_data_stub);
  zwave_tx_send_data_ExpectWithArrayAndReturn(&connection_info_1,
                                              sizeof(connection_info_1),
                                              sizeof(frame_data),
                                              frame_data,
                                              sizeof(frame_data),
                                              &tx_options_1,
                                              sizeof(tx_options_1),
                                              NULL,
                                              NULL,
                                              0,
                                              NULL,
                                              0,
                                              SL_STATUS_OK);
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_session();

  TEST_ASSERT_NOT_NULL(received_resolver_config.send);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    received_resolver_config.send(test_node_2,
                                                  frame_data,
                                                  sizeof(frame_data),
                                                  is_set));

  // Sending has been triggered. Now the callback.
  attribute_store_get_node_type_ExpectAndReturn(test_node_2, test_node_2_type);
  on_resolver_send_data_complete_Expect(RESOLVER_SEND_STATUS_OK,
                                        0,
                                        test_node_2,
                                        RESOLVER_SET_RULE);
  TEST_ASSERT_NOT_NULL(received_send_data_complete);
  received_send_data_complete(TRANSMIT_COMPLETE_OK, NULL, received_user);

  // Reset the pending resolution of test_node_2
  received_resolver_config.send_init();
}

void test_zpc_attribute_resolver_register_custom_handler()
{
  attribute_store_node_t test_type = 6;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    register_send_event_handler(test_type, custom_send_event_handler));

  TEST_ASSERT_EQUAL(SL_STATUS_ALREADY_EXISTS,
                    register_send_event_handler(test_type, NULL));

  attribute_store_node_t test_node = 75;
  const uint8_t frame_data[]       = {0x01, 0x01, 0xFF, 0x00, 0x0A, 0x04};
  bool is_set                      = true;

  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    test_node,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_node_id(
    &zwave_node_id_1);
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &zwave_endpoint_id_1);

  zwave_tx_scheme_get_node_connection_info_Expect(zwave_node_id_1,
                                                  zwave_endpoint_id_1,
                                                  NULL);
  zwave_tx_scheme_get_node_connection_info_IgnoreArg_connection_info();
  zwave_tx_scheme_get_node_connection_info_ReturnThruPtr_connection_info(
    &connection_info_1);

  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_NODE_INTERVIEW_PRIORITY,
    0,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();
  zwave_tx_scheme_get_node_tx_options_ReturnThruPtr_tx_options(&tx_options_1);

  zwave_node_supports_command_class_ExpectAndReturn(0x6C,
                                                    zwave_node_id_1,
                                                    zwave_endpoint_id_1,
                                                    false);

  zwave_tx_send_data_AddCallback(&zwave_tx_send_data_stub);
  zwave_tx_send_data_ExpectWithArrayAndReturn(&connection_info_1,
                                              sizeof(connection_info_1),
                                              sizeof(frame_data),
                                              frame_data,
                                              sizeof(frame_data),
                                              &tx_options_1,
                                              sizeof(tx_options_1),
                                              NULL,
                                              NULL,
                                              0,
                                              NULL,
                                              0,
                                              SL_STATUS_OK);
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_session();

  TEST_ASSERT_NOT_NULL(received_resolver_config.send);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    received_resolver_config.send(test_node,
                                                  frame_data,
                                                  sizeof(frame_data),
                                                  is_set));

  // Sending has been triggered. Now the callback.
  attribute_store_get_node_type_ExpectAndReturn(test_node, test_type);
  on_resolver_send_data_complete_Expect(RESOLVER_SEND_STATUS_ALREADY_HANDLED,
                                        0,
                                        test_node,
                                        RESOLVER_SET_RULE);
  TEST_ASSERT_NOT_NULL(received_send_data_complete);
  TEST_ASSERT_EQUAL(0, custom_handler_calls);
  received_send_data_complete(TRANSMIT_COMPLETE_OK, NULL, received_user);
  TEST_ASSERT_EQUAL(1, custom_handler_calls);
}

void test_zpc_attribute_resolver_send_set_abort_pending()
{
  attribute_store_node_t test_node      = 28647;
  attribute_store_type_t test_type      = 382;
  zwave_tx_session_id_t test_session_id = (void *)0x998877;
  TEST_ASSERT_NOT_NULL(set_notification_function);
  TEST_ASSERT_NOT_NULL(received_resolver_config.abort);

  // Try to abort when nothing is going on:
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    received_resolver_config.abort(test_node));

  // Notify of incoming set resolution, but only 1 node will just be ignored.
  attribute_store_is_value_defined_ExpectAndReturn(test_node,
                                                   DESIRED_ATTRIBUTE,
                                                   true);
  attribute_store_is_value_matched_ExpectAndReturn(test_node, false);
  set_notification_function(test_node, ATTRIBUTE_UPDATED);

  const uint8_t frame_data[] = {0x01, 0x01, 0xFF, 0x00, 0x0A, 0x04};
  bool is_set                = true;

  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    test_node,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_node_id(
    &zwave_node_id_1);
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &zwave_endpoint_id_1);

  zwave_tx_scheme_get_node_connection_info_Expect(zwave_node_id_1,
                                                  zwave_endpoint_id_1,
                                                  NULL);
  zwave_tx_scheme_get_node_connection_info_IgnoreArg_connection_info();
  zwave_tx_scheme_get_node_connection_info_ReturnThruPtr_connection_info(
    &connection_info_1);

  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_NODE_INTERVIEW_PRIORITY,
    0,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();
  zwave_tx_scheme_get_node_tx_options_ReturnThruPtr_tx_options(&tx_options_1);

  zwave_node_supports_command_class_ExpectAndReturn(0x6C,
                                                    zwave_node_id_1,
                                                    zwave_endpoint_id_1,
                                                    true);

  zwave_command_class_supervision_send_data_AddCallback(
    &zwave_tx_send_data_stub);
  zwave_command_class_supervision_send_data_ExpectWithArrayAndReturn(
    &connection_info_1,
    sizeof(connection_info_1),
    sizeof(frame_data),
    frame_data,
    sizeof(frame_data),
    &tx_options_1,
    sizeof(tx_options_1),
    NULL,
    NULL,
    0,
    NULL,
    0,
    SL_STATUS_OK);
  zwave_command_class_supervision_send_data_IgnoreArg_on_supervision_complete();
  zwave_command_class_supervision_send_data_IgnoreArg_user();
  zwave_command_class_supervision_send_data_IgnoreArg_session();
  zwave_command_class_supervision_send_data_ReturnThruPtr_session(
    &test_session_id);

  TEST_ASSERT_NOT_NULL(received_resolver_config.send);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    received_resolver_config.send(test_node,
                                                  frame_data,
                                                  sizeof(frame_data),
                                                  is_set));

  // Sending has been triggered. Now the callback.
  attribute_store_get_node_type_ExpectAndReturn(test_node, test_type);
  on_resolver_send_data_complete_Expect(
    RESOLVER_SEND_STATUS_OK_EXECUTION_PENDING,
    0,
    test_node,
    RESOLVER_SET_RULE);
  TEST_ASSERT_NOT_NULL(received_send_data_complete);
  received_send_data_complete(SUPERVISION_REPORT_WORKING, NULL, received_user);

  // Now we want to abort it.
  zwave_command_class_supervision_close_session_by_tx_session_ExpectAndReturn(
    test_session_id,
    SL_STATUS_OK);
  on_resolver_send_data_complete_Expect(RESOLVER_SEND_STATUS_ABORTED,
                                        0,
                                        test_node,
                                        RESOLVER_SET_RULE);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, received_resolver_config.abort(test_node));

  // If we get a bogus callback, it should be ignored:
  received_send_data_complete(SUPERVISION_REPORT_SUCCESS, NULL, received_user);
}
