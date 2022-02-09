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
// Includes from this component
#include "zwave_command_class_multi_channel_association.h"
#include "zwave_command_class_association_internals.h"  // to use the frame defs
#include "zwave_command_class_agi.h"
#include "zwave_command_classes_utils.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "unity.h"
#include "sl_log.h"
#include "ZW_classcmd.h"

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "zwave_unid.h"
#include "zwave_controller_utils.h"

// Interface includes
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zwave_utils.h"
#include "zwave_controller_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "attribute_resolver_mock.h"
#include "zpc_attribute_resolver_mock.h"
#include "zwave_tx_scheme_selector_mock.h"
#include "zwave_command_handler_mock.h"
#include "zwave_network_management_mock.h"
#include "zwave_tx_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_##type

// Static variables
static attribute_resolver_function_t multi_channel_association_set = NULL;
static attribute_resolver_function_t multi_channel_association_get = NULL;
static attribute_resolver_function_t multi_channel_association_groupings_get
  = NULL;
static zpc_resolver_event_notification_function_t on_send_complete = NULL;
static zwave_command_handler_t multi_channel_association_handler   = {};

static zwave_controller_connection_info_t connection = {};

static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;

static uint8_t received_data[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
static uint8_t received_data_size                                  = 0;
static uint8_t u8_value                                            = 0;

// Stub functions
static sl_status_t zwave_tx_send_data_stub(
  const zwave_controller_connection_info_t *connection,
  uint16_t frame_length,
  const uint8_t *frame_data,
  const zwave_tx_options_t *tx_options,
  const on_zwave_tx_send_data_complete_t on_send_complete,
  void *user,
  zwave_tx_session_id_t *session,
  int cmock_num_calls)
{
  memcpy(received_frame, frame_data, frame_length);
  received_frame_size = frame_length;
  return SL_STATUS_OK;
}

static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  if (node_type != ATTRIBUTE(SUPPORTED_GROUPINGS)
      && node_type != ATTRIBUTE(GROUP_CONTENT)) {
    TEST_FAIL_MESSAGE("Attribute rule registration on a wrong type");
  }

  if (node_type == ATTRIBUTE(SUPPORTED_GROUPINGS)) {
    TEST_ASSERT_NULL(set_func);
    multi_channel_association_groupings_get = get_func;
  } else if (node_type == ATTRIBUTE(GROUP_CONTENT)) {
    multi_channel_association_set = set_func;
    multi_channel_association_get = get_func;
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  multi_channel_association_handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME,
                    multi_channel_association_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3,
                    multi_channel_association_handler.command_class);
  TEST_ASSERT_EQUAL(MULTI_CHANNEL_ASSOCIATION_VERSION_V3,
                    multi_channel_association_handler.version);
  TEST_ASSERT_NOT_NULL(multi_channel_association_handler.control_handler);
  TEST_ASSERT_NOT_NULL(multi_channel_association_handler.support_handler);
  TEST_ASSERT_FALSE(
    multi_channel_association_handler.manual_security_validation);

  return SL_STATUS_OK;
}

static sl_status_t register_send_event_handler_stub(
  attribute_store_type_t type,
  zpc_resolver_event_notification_function_t function,
  int cmock_num_calls)
{
  on_send_complete = function;
  TEST_ASSERT_NOT_NULL(on_send_complete);
  TEST_ASSERT_EQUAL(ATTRIBUTE(GROUP_CONTENT), type);
  return SL_STATUS_OK;
}

static void init_verification()
{
  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // On send complete handler
  register_send_event_handler_Stub(&register_send_event_handler_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  // TX interceptions
  zwave_tx_send_data_AddCallback(&zwave_tx_send_data_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_multi_channel_association_init());

  // De - activate the stubs not to overwrite the data from init if other CCs are initialized
  attribute_resolver_register_rule_Stub(NULL);
  register_send_event_handler_Stub(NULL);
  zwave_command_handler_register_handler_Stub(NULL);
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zwave_unid_set_home_id(home_id);
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);
  zwave_network_management_get_node_id_IgnoreAndReturn(zpc_node_id);
  zpc_attribute_store_test_helper_create_network();
  init_verification();
  zwave_unid_set_home_id(home_id);

  memset(&connection, 0, sizeof(connection));
  memset(received_frame, 0, sizeof(received_frame));
  memset(received_data, 0, sizeof(received_data));
  received_frame_size = 0;
  received_data_size  = 0;
}

void test_zwave_command_class_multi_channel_association_support()
{
  // Get the AGI to initialize our groups.
  attribute_resolver_register_rule_IgnoreAndReturn(SL_STATUS_OK);
  zwave_command_handler_register_handler_IgnoreAndReturn(SL_STATUS_OK);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_agi_init());

  // Receive a Set Command:
  // The last byte 0x12 will be ignored.
  const association_group_id_t received_group = 1;
  const association_set_frame_t set
    = {.command_class       = COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3,
       .command             = MULTI_CHANNEL_ASSOCIATION_SET_V3,
       .grouping_identifier = (uint8_t)received_group,
       .group_content       = {0x01, 0x00, 0x02, 0x03, 0x12}};
  connection.remote.endpoint_id = endpoint_id;
  connection.remote.node_id     = node_id;
  TEST_ASSERT_NOT_NULL(multi_channel_association_handler.support_handler);
  multi_channel_association_handler.support_handler(&connection,
                                                    (uint8_t *)&set,
                                                    8);

  // Verify that it's according to our expectation:
  attribute_store_node_t group_id_node
    = attribute_store_get_node_child_by_value(zpc_endpoint_id_node,
                                              ATTRIBUTE(GROUP_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&received_group,
                                              sizeof(association_group_id_t),
                                              0);
  attribute_store_node_t group_content_node
    = attribute_store_get_node_child_by_type(group_id_node,
                                             ATTRIBUTE(GROUP_CONTENT),
                                             0);
  const uint8_t expected_associations_1[] = {0x01, 0x00, 0x02, 0x03};
  attribute_store_get_reported(group_content_node,
                               received_data,
                               sizeof(expected_associations_1));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_associations_1,
                                received_data,
                                sizeof(expected_associations_1));

  // Receive a Remove Command:
  // The last byte 0x23 will be ignored.
  const association_set_frame_t remove
    = {.command_class       = COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3,
       .command             = MULTI_CHANNEL_ASSOCIATION_REMOVE_V3,
       .grouping_identifier = 01,
       .group_content       = {0x02, 0x00, 0x02, 0x03, 0x23}};

  TEST_ASSERT_NOT_NULL(multi_channel_association_handler.support_handler);
  multi_channel_association_handler.support_handler(&connection,
                                                    (uint8_t *)&remove,
                                                    8);

  const uint8_t expected_associations_2[] = {0x01};
  attribute_store_get_reported(group_content_node,
                               received_data,
                               sizeof(expected_associations_2));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_associations_2,
                                received_data,
                                sizeof(expected_associations_2));

  // Receive a Groupings Get Command:
  const zwave_minimum_frame_t group_get_frame
    = {.command_class = COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3,
       .command       = MULTI_CHANNEL_ASSOCIATION_GROUPINGS_GET_V3};

  // We expect a TX queue message in return, containing the value retrieved
  // from the attribute store
  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY,
    0,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();

  attribute_store_node_t supported_groupings_node
    = attribute_store_get_node_child_by_type(zpc_endpoint_id_node,
                                             ATTRIBUTE(SUPPORTED_GROUPINGS),
                                             0);
  association_group_count_t supported_groupings = 0;
  attribute_store_get_reported(supported_groupings_node,
                               &supported_groupings,
                               sizeof(supported_groupings));
  const association_grouping_report_frame_t expected_report_1
    = {.command_class       = COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3,
       .command             = MULTI_CHANNEL_ASSOCIATION_GROUPINGS_REPORT_V3,
       .supported_groupings = (uint8_t)supported_groupings};

  zwave_tx_send_data_ExpectAndReturn(NULL,
                                     sizeof(expected_report_1),
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);
  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_IgnoreArg_tx_options();

  TEST_ASSERT_NOT_NULL(multi_channel_association_handler.support_handler);
  multi_channel_association_handler.support_handler(&connection,
                                                    (uint8_t *)&group_get_frame,
                                                    sizeof(group_get_frame));

  //Verify that the frame we received is matching:
  TEST_ASSERT_EQUAL(sizeof(expected_report_1), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY((uint8_t *)&expected_report_1,
                                received_frame,
                                received_frame_size);

  // Receive a Multi Channel Association Get Command:
  ZW_MULTI_CHANNEL_ASSOCIATION_GET_V3_FRAME get_frame
    = {.cmdClass           = COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3,
       .cmd                = MULTI_CHANNEL_ASSOCIATION_GET_V3,
       .groupingIdentifier = 01};

  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY,
    0,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();

  attribute_store_node_t group_capacity_node
    = attribute_store_get_node_child_by_type(group_id_node,
                                             ATTRIBUTE(MAX_NODES_SUPPORTED),
                                             0);
  association_group_capacity_t expected_capacity = 0;
  attribute_store_get_reported(group_capacity_node,
                               &expected_capacity,
                               sizeof(expected_capacity));
  const association_report_frame_t expected_report_2
    = {.command_class       = COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3,
       .command             = MULTI_CHANNEL_ASSOCIATION_REPORT_V3,
       .grouping_identifier = 01,
       .max_nodes_supported = expected_capacity,
       .reports_to_follow   = RESERVED_BYTE,
       .group_content       = {0x01}};
  zwave_tx_send_data_ExpectAndReturn(NULL,
                                     6,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);
  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_IgnoreArg_tx_options();

  TEST_ASSERT_NOT_NULL(multi_channel_association_handler.support_handler);
  multi_channel_association_handler.support_handler(&connection,
                                                    (uint8_t *)&get_frame,
                                                    sizeof(get_frame));

  //Verify that the frame we received is matching:
  TEST_ASSERT_EQUAL(6, received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY((uint8_t *)&expected_report_2,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_multi_channel_association_control()
{
  // Add the version node:
  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(endpoint_id_node));
  attribute_store_node_t version_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_VERSION,
    endpoint_id_node);

  // No new attribute created due to unresolved version.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  u8_value = 0;
  attribute_store_set_reported(version_node, &u8_value, sizeof(u8_value));
  // No new attribute created due to version = 0.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  u8_value = 2;
  attribute_store_set_reported(version_node, &u8_value, sizeof(u8_value));
  // SUPPORTED_GROUPINGS Attribute got created:
  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(endpoint_id_node));

  attribute_store_node_t groupings_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SUPPORTED_GROUPINGS),
                                             0);

  // Check that we can resolve supporting groupings:
  // First check the frame creation:
  TEST_ASSERT_NOT_NULL(multi_channel_association_groupings_get);
  multi_channel_association_groupings_get(groupings_node,
                                          received_frame,
                                          &received_frame_size);
  const uint8_t expected_frame_1[]
    = {COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3,
       MULTI_CHANNEL_ASSOCIATION_GROUPINGS_GET_V3};
  TEST_ASSERT_EQUAL(sizeof(expected_frame_1), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame_1,
                                received_frame,
                                received_frame_size);

  // Response for Groupings get:
  const association_grouping_report_frame_t grouping_report
    = {.command_class       = COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3,
       .command             = MULTI_CHANNEL_ASSOCIATION_GROUPINGS_REPORT_V3,
       .supported_groupings = 2};
  connection.remote.endpoint_id = endpoint_id;
  connection.remote.node_id     = node_id;
  TEST_ASSERT_NOT_NULL(multi_channel_association_handler.control_handler);
  multi_channel_association_handler.control_handler(&connection,
                                                    (uint8_t *)&grouping_report,
                                                    sizeof(grouping_report));

  // Verify that the value was accepted and saved in the Attribute Store
  association_group_count_t groupings = 0;
  attribute_store_get_reported(groupings_node, &groupings, sizeof(groupings));
  TEST_ASSERT_EQUAL(2, groupings);

  // Check that we can resolve group contents:
  const association_group_id_t group_1 = 1;
  attribute_store_node_t group_1_node
    = attribute_store_get_node_child_by_value(endpoint_id_node,
                                              ATTRIBUTE(GROUP_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&group_1,
                                              sizeof(group_1),
                                              0);
  attribute_store_node_t group_content_node
    = attribute_store_get_node_child_by_type(group_1_node,
                                             ATTRIBUTE(GROUP_CONTENT),
                                             0);
  // First check the frame creation (Get):
  TEST_ASSERT_NOT_NULL(multi_channel_association_get);
  multi_channel_association_get(group_content_node,
                                received_frame,
                                &received_frame_size);
  const uint8_t expected_frame_2[]
    = {COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3,
       MULTI_CHANNEL_ASSOCIATION_GET_V3,
       (uint8_t)group_1};
  TEST_ASSERT_EQUAL(sizeof(expected_frame_2), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame_2,
                                received_frame,
                                received_frame_size);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(group_content_node, REPORTED_ATTRIBUTE));

  // Simulate a response for Association get:
  const association_report_frame_t report
    = {.command_class       = COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3,
       .command             = MULTI_CHANNEL_ASSOCIATION_REPORT_V3,
       .grouping_identifier = 1,
       .max_nodes_supported = 3,
       .reports_to_follow   = 0x00,
       .group_content       = {02, 03, 00, 04, 01}};
  connection.remote.endpoint_id = endpoint_id;
  connection.remote.node_id     = node_id;

  TEST_ASSERT_NOT_NULL(multi_channel_association_handler.control_handler);
  multi_channel_association_handler.control_handler(&connection,
                                                    (uint8_t *)&report,
                                                    sizeof(report));

  // Now go check the group data in the attribute store:
  attribute_store_get_node_attribute_value(group_content_node,
                                           REPORTED_ATTRIBUTE,
                                           received_data,
                                           &received_data_size);
  TEST_ASSERT_EQUAL(5, received_data_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(report.group_content,
                                received_data,
                                received_data_size);

  attribute_store_node_t group_capacity_node
    = attribute_store_get_node_child_by_type(group_1_node,
                                             ATTRIBUTE(MAX_NODES_SUPPORTED),
                                             0);
  association_group_capacity_t saved_group_capacity = 0;
  attribute_store_get_reported(group_capacity_node,
                               (uint8_t *)&saved_group_capacity,
                               sizeof(saved_group_capacity));
  TEST_ASSERT_EQUAL(report.max_nodes_supported, saved_group_capacity);
  zwave_network_management_assign_return_route_IgnoreAndReturn(SL_STATUS_OK);
  // Now trigger a Lifeline establishement.
  // First shot, nothing happens due to no AGI profile data.
  attribute_resolver_clear_resolution_listener_Expect(
    endpoint_id_node,
    &establish_lifeline_association);
  establish_lifeline_association(endpoint_id_node);

  // Now set a group with a Profile 1:
  attribute_store_node_t group_profile_node
    = attribute_store_get_node_child_by_type(group_1_node,
                                             ATTRIBUTE(GROUP_PROFILE),
                                             0);
  const agi_profile_t lifeline_profile = AGI_LIFELINE_PROFILE;
  attribute_store_set_reported(group_profile_node,
                               &lifeline_profile,
                               sizeof(lifeline_profile));
  // Second shot, we are not the SIS so we will fail:
  attribute_resolver_clear_resolution_listener_Expect(
    endpoint_id_node,
    &establish_lifeline_association);
  zwave_network_management_is_zpc_sis_ExpectAndReturn(false);
  establish_lifeline_association(endpoint_id_node);
  // Now go check the group data in the attribute store:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(group_content_node, DESIRED_ATTRIBUTE));
  // Third time the charm, we bump somebody out!
  attribute_resolver_clear_resolution_listener_Expect(
    endpoint_id_node,
    &establish_lifeline_association);
  zwave_network_management_is_zpc_sis_ExpectAndReturn(true);
  establish_lifeline_association(endpoint_id_node);
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(group_content_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(attribute_store_is_value_matched(group_content_node));

  // Try to get the command for solving the mismatch:
  // We should first remove node 02 and then add 01
  TEST_ASSERT_NOT_NULL(multi_channel_association_set);
  multi_channel_association_set(group_content_node,
                                received_frame,
                                &received_frame_size);
  const uint8_t expected_frame_3[]
    = {COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3,
       MULTI_CHANNEL_ASSOCIATION_REMOVE_V3,
       group_1,
       0x02};
  TEST_ASSERT_EQUAL(sizeof(expected_frame_3), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame_3,
                                received_frame,
                                received_frame_size);

  // Pretend it worked by updating the reported value of the attribute store
  received_data[0] = 3;
  received_data[1] = 0;
  received_data[2] = 4;
  received_data[3] = 1;
  attribute_store_set_node_attribute_value(group_content_node,
                                           REPORTED_ATTRIBUTE,
                                           received_data,
                                           4);

  // Now the frame handler should try to add node ID 1
  TEST_ASSERT_NOT_NULL(multi_channel_association_set);
  multi_channel_association_set(group_content_node,
                                received_frame,
                                &received_frame_size);
  const uint8_t expected_frame_4[]
    = {COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3,
       MULTI_CHANNEL_ASSOCIATION_SET_V3,
       group_1,
       zpc_node_id};
  TEST_ASSERT_EQUAL(sizeof(expected_frame_4), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame_4,
                                received_frame,
                                received_frame_size);

  // Test that we undefine attributes after a send data complete:
  TEST_ASSERT_NOT_NULL(on_send_complete);
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(group_content_node, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(group_content_node, DESIRED_ATTRIBUTE));

  on_send_complete(group_content_node,
                   RESOLVER_SET_RULE,
                   FRAME_SENT_EVENT_OK_SUPERVISION_WORKING);
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(group_content_node, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(group_content_node, DESIRED_ATTRIBUTE));

  on_send_complete(group_content_node,
                   RESOLVER_GET_RULE,
                   FRAME_SENT_EVENT_OK_SUPERVISION_NO_SUPPORT);
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(group_content_node, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(group_content_node, DESIRED_ATTRIBUTE));

  on_send_complete(group_content_node,
                   RESOLVER_SET_RULE,
                   FRAME_SENT_EVENT_OK_NO_SUPERVISION);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(group_content_node, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(group_content_node, DESIRED_ATTRIBUTE));

  on_send_complete(group_content_node,
                   RESOLVER_SET_RULE,
                   FRAME_SENT_EVENT_OK_SUPERVISION_FAIL);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(group_content_node, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(group_content_node, DESIRED_ATTRIBUTE));
}

void test_zwave_command_class_multi_channel_association_association_to_relevant_groups()
{
  // We want to get associate ourselves to these groups:
  zwave_command_class_agi_request_to_establish_association(0x33, 0x33);

  attribute_store_node_t group_id_node
    = attribute_store_add_node(ATTRIBUTE(GROUP_ID), endpoint_id_node);
  association_group_id_t group_id = 3;
  attribute_store_set_reported(group_id_node, &group_id, sizeof(group_id));

  uint8_t group_command_list[] = {1, 2, 1, 4, 0x33, 0x33};
  attribute_store_node_t group_command_list_node
    = attribute_store_get_node_child_by_type(group_id_node,
                                             ATTRIBUTE(GROUP_COMMAND_LIST),
                                             0);
  attribute_store_set_reported(group_command_list_node,
                               group_command_list,
                               sizeof(group_command_list));
  attribute_store_node_t supported_groupings_node
    = attribute_store_add_node(ATTRIBUTE(SUPPORTED_GROUPINGS),
                               endpoint_id_node);

  association_group_count_t number_of_groups = 3;
  attribute_store_set_reported(supported_groupings_node,
                               &number_of_groups,
                               sizeof(number_of_groups));

  // Set the group profile to something else than NA or Lifeline
  attribute_store_node_t group_profile_node
    = attribute_store_get_node_child_by_type(group_id_node,
                                             ATTRIBUTE(GROUP_PROFILE),
                                             0);
  const agi_profile_t group_profile = 0x2020;
  attribute_store_set_reported(group_profile_node,
                               &group_profile,
                               sizeof(group_profile));
  attribute_store_node_t group_capacity_node
    = attribute_store_add_node(ATTRIBUTE(MAX_NODES_SUPPORTED), group_id_node);
  association_group_capacity_t group_capacity = 20;
  attribute_store_set_reported(group_capacity_node,
                               &group_capacity,
                               sizeof(group_capacity));

  // Check that AGI likes the group content:
  TEST_ASSERT_TRUE(
    zwave_command_class_agi_group_contains_listeners(node_id, endpoint_id, 3));

  // Make sure the group content exists
  attribute_store_node_t group_content_node
    = attribute_store_add_node(ATTRIBUTE(GROUP_CONTENT), group_id_node);

  //Trigger lifeline establishment
  attribute_resolver_clear_resolution_listener_Expect(
    endpoint_id_node,
    &establish_lifeline_association);
  zwave_network_management_is_zpc_sis_ExpectAndReturn(true);
  zwave_network_management_assign_return_route_IgnoreAndReturn(SL_STATUS_OK);
  establish_lifeline_association(endpoint_id_node);

  // Verify the desired value:
  const uint8_t expected_associations[] = {0x01};
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(group_content_node, DESIRED_ATTRIBUTE));
  uint8_t actual_associations[sizeof(expected_associations)] = {};
  attribute_store_get_desired(group_content_node,
                              actual_associations,
                              sizeof(actual_associations));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_associations,
                                actual_associations,
                                sizeof(expected_associations));
}