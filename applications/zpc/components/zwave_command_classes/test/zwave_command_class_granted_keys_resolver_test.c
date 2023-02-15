/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "zwave_command_class_granted_keys_resolver.h"
#include "unity.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "zpc_attribute_store_type_registration.h"

// Interface includes
#include "attribute_store_defined_attribute_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "attribute_resolver_mock.h"
#include "zpc_attribute_resolver_mock.h"

// Static variables
static attribute_resolver_function_t get_granted_keys                    = NULL;
static attribute_resolver_function_t get_inclusion_protocol              = NULL;
static attribute_resolver_function_t probe_key_and_protocol              = NULL;
static zpc_resolver_event_notification_function_t on_probe_send_complete = NULL;

static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;

// Stub functions
static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  TEST_ASSERT_NULL(set_func);
  if (node_type == ATTRIBUTE_GRANTED_SECURITY_KEYS) {
    get_granted_keys = get_func;
  } else if (node_type == ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL) {
    get_inclusion_protocol = get_func;
  } else if (node_type == ATTRIBUTE_ZWAVE_KEY_AND_PROTOCOL_TO_DISCOVER) {
    probe_key_and_protocol = get_func;
  } else {
    TEST_FAIL_MESSAGE("Attribute rule registration on a wrong type");
  }
  return SL_STATUS_OK;
}

sl_status_t register_send_event_handler_stub(
  attribute_store_type_t type,
  zpc_resolver_event_notification_function_t function,
  int cmock_num_calls)
{
  if (type == ATTRIBUTE_ZWAVE_KEY_AND_PROTOCOL_TO_DISCOVER) {
    on_probe_send_complete = function;
  } else {
    TEST_FAIL_MESSAGE("Send event handler registration on a wrong type");
  }
  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zpc_attribute_store_register_known_attribute_types();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

static void init_verification()
{
  // Rule registration
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // ZPC send event handler:
  register_send_event_handler_Stub(&register_send_event_handler_stub);

  get_granted_keys       = NULL;
  get_inclusion_protocol = NULL;
  probe_key_and_protocol = NULL;
  on_probe_send_complete = NULL;

  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size = 0;

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_granted_keys_resolver_init());
}

/// Called before each and every test
void setUp()
{
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_set_home_id(home_id);

  // Change the supporting node to have Endpoint ID 0 instead of 4
  zwave_endpoint_id_t id = 0;
  attribute_store_set_reported(endpoint_id_node, &id, sizeof(id));

  init_verification();
}

void test_get_granted_keys_resolution_function_waiting_for_nif()
{
  attribute_store_node_t keyset_node
    = attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS, node_id_node);

  TEST_ASSERT_EQUAL(
    SL_STATUS_IS_WAITING,
    get_granted_keys(keyset_node, received_frame, &received_frame_size));

  TEST_ASSERT_EQUAL(0, received_frame_size);
}

void test_get_granted_keys_resolution_non_secure_node()
{
  TEST_ASSERT_NOT_NULL(get_granted_keys);
  attribute_store_node_t keyset_node
    = attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS, node_id_node);

  attribute_store_node_t nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);
  const uint8_t nif_content[] = {1, 23, 4};
  attribute_resolver_restart_get_resolution_ExpectAndReturn(keyset_node,
                                                            SL_STATUS_OK);
  attribute_resolver_restart_get_resolution_ExpectAndReturn(0, SL_STATUS_OK);
  attribute_store_set_reported(nif_node, nif_content, sizeof(nif_content));

  TEST_ASSERT_EQUAL(
    SL_STATUS_ALREADY_EXISTS,
    get_granted_keys(keyset_node, received_frame, &received_frame_size));

  TEST_ASSERT_EQUAL(0, received_frame_size);
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(keyset_node, REPORTED_ATTRIBUTE));
}

void test_get_granted_keys_resolution_happy_case()
{
  TEST_ASSERT_NOT_NULL(get_granted_keys);
  attribute_store_node_t keyset_node
    = attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS, node_id_node);

  attribute_store_node_t nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);
  const uint8_t nif_content[] = {0x9F, 3, 4};
  attribute_resolver_restart_get_resolution_ExpectAndReturn(keyset_node,
                                                            SL_STATUS_OK);
  attribute_resolver_restart_get_resolution_ExpectAndReturn(0, SL_STATUS_OK);
  attribute_store_set_reported(nif_node, nif_content, sizeof(nif_content));

  TEST_ASSERT_EQUAL(
    SL_STATUS_IS_WAITING,
    get_granted_keys(keyset_node, received_frame, &received_frame_size));

  TEST_ASSERT_EQUAL(0, received_frame_size);
  // Should have created 5 key/protocol combinations under the endpoint id 0 node.
  // NIF + 5 new attributes
  TEST_ASSERT_EQUAL(6, attribute_store_get_node_child_count(endpoint_id_node));
}

void test_get_inclusion_protocol_resolution_function_waiting_for_nif()
{
  TEST_ASSERT_NOT_NULL(get_inclusion_protocol);
  attribute_store_node_t inclusion_protocol_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                               node_id_node);

  TEST_ASSERT_EQUAL(SL_STATUS_IS_WAITING,
                    get_inclusion_protocol(inclusion_protocol_node,
                                           received_frame,
                                           &received_frame_size));

  TEST_ASSERT_EQUAL(0, received_frame_size);
}

void test_get_inclusion_protocol_resolution_non_secure_node()
{
  TEST_ASSERT_NOT_NULL(get_inclusion_protocol);
  attribute_store_node_t inclusion_protocol_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                               node_id_node);

  attribute_store_node_t nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);
  const uint8_t nif_content[] = {1, 23, 4};
  attribute_resolver_restart_get_resolution_ExpectAndReturn(0, SL_STATUS_OK);
  attribute_resolver_restart_get_resolution_ExpectAndReturn(
    inclusion_protocol_node,
    SL_STATUS_OK);
  attribute_store_set_reported(nif_node, nif_content, sizeof(nif_content));

  TEST_ASSERT_EQUAL(SL_STATUS_ALREADY_EXISTS,
                    get_inclusion_protocol(inclusion_protocol_node,
                                           received_frame,
                                           &received_frame_size));

  TEST_ASSERT_EQUAL(0, received_frame_size);
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(inclusion_protocol_node,
                                                    REPORTED_ATTRIBUTE));
}

void test_get_inclusion_protocol_resolution_happy_case()
{
  TEST_ASSERT_NOT_NULL(get_inclusion_protocol);
  attribute_store_node_t inclusion_protocol_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                               node_id_node);

  attribute_store_node_t nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);
  const uint8_t nif_content[] = {0x98, 3, 4};
  attribute_resolver_restart_get_resolution_ExpectAndReturn(0, SL_STATUS_OK);
  attribute_resolver_restart_get_resolution_ExpectAndReturn(
    inclusion_protocol_node,
    SL_STATUS_OK);
  attribute_store_set_reported(nif_node, nif_content, sizeof(nif_content));

  TEST_ASSERT_EQUAL(SL_STATUS_IS_WAITING,
                    get_inclusion_protocol(inclusion_protocol_node,
                                           received_frame,
                                           &received_frame_size));

  TEST_ASSERT_EQUAL(0, received_frame_size);
  // Should have created 1 key/protocol combinations (only S0) under the
  // endpoint id 0 node.
  // NIF + 1 new attributes
  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(endpoint_id_node));
}

void test_probe_key_and_protocol_s2_happy_case()
{
  TEST_ASSERT_NOT_NULL(probe_key_and_protocol);

  attribute_store_node_t combination_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_KEY_AND_PROTOCOL_TO_DISCOVER,
                               endpoint_id_node);

  zwave_key_protocol_combination_t combination
    = {.key      = ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY,
       .protocol = PROTOCOL_ZWAVE_LONG_RANGE};
  attribute_store_set_desired(combination_node,
                              &combination,
                              sizeof(combination));
  attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS, node_id_node);
  attribute_store_node_t inclusion_protocol_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                               node_id_node);

  attribute_resolver_pause_node_resolution_Expect(node_id_node);
  TEST_ASSERT_EQUAL(SL_STATUS_IS_WAITING,
                    probe_key_and_protocol(combination_node,
                                           received_frame,
                                           &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_SECURITY_2, SECURITY_2_COMMANDS_SUPPORTED_GET};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);

  zwave_protocol_t desired_protocol = 0xFF;
  attribute_store_get_desired(inclusion_protocol_node,
                              &desired_protocol,
                              sizeof(desired_protocol));
  TEST_ASSERT_EQUAL(combination.protocol, desired_protocol);
}

void test_probe_key_and_protocol_s0_happy_case()
{
  TEST_ASSERT_NOT_NULL(probe_key_and_protocol);

  attribute_store_node_t combination_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_KEY_AND_PROTOCOL_TO_DISCOVER,
                               endpoint_id_node);

  zwave_key_protocol_combination_t combination
    = {.key = ZWAVE_CONTROLLER_S0_KEY, .protocol = PROTOCOL_ZWAVE_LONG_RANGE};
  attribute_store_set_desired(combination_node,
                              &combination,
                              sizeof(combination));

  attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS, node_id_node);
  attribute_store_node_t inclusion_protocol_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                               node_id_node);

  attribute_resolver_pause_node_resolution_Expect(node_id_node);
  TEST_ASSERT_EQUAL(SL_STATUS_IS_WAITING,
                    probe_key_and_protocol(combination_node,
                                           received_frame,
                                           &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_SECURITY, SECURITY_COMMANDS_SUPPORTED_GET};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);

  zwave_protocol_t desired_protocol = 0xFF;
  attribute_store_get_desired(inclusion_protocol_node,
                              &desired_protocol,
                              sizeof(desired_protocol));
  TEST_ASSERT_EQUAL(combination.protocol, desired_protocol);
}

void test_mark_protocol_as_supported()
{
  zwave_key_protocol_combination_t combination
    = {.key = ZWAVE_CONTROLLER_S0_KEY, .protocol = PROTOCOL_ZWAVE_LONG_RANGE};

  attribute_store_node_t granted_keys_node
    = attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS, node_id_node);
  attribute_store_node_t inclusion_protocol_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                               node_id_node);

  attribute_store_set_desired(inclusion_protocol_node,
                              &combination.protocol,
                              sizeof(combination.protocol));
  const zwave_keyset_t already_granted = ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY;
  attribute_store_set_reported(granted_keys_node,
                               &already_granted,
                               sizeof(already_granted));

  //Mark the protocol/key as supported, so it gets added to the reported values:
  zwave_command_class_mark_key_protocol_as_supported(
    node_id_node,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0);

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(granted_keys_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(inclusion_protocol_node,
                                                     DESIRED_ATTRIBUTE));
  zwave_keyset_t new_keyset = 0;
  attribute_store_get_reported(granted_keys_node,
                               &new_keyset,
                               sizeof(new_keyset));
  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY
                      | ZWAVE_CONTROLLER_S0_KEY,
                    new_keyset);

  zwave_protocol_t new_protocol = PROTOCOL_UNKNOWN;
  attribute_store_get_reported(inclusion_protocol_node,
                               &new_protocol,
                               sizeof(new_protocol));

  TEST_ASSERT_EQUAL(combination.protocol, new_protocol);
}

void test_mark_protocol_as_zwave_for_non_secure_nodes()
{
  attribute_store_node_t granted_keys_node
    = attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS, node_id_node);

  zwave_keyset_t granted_keys = ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY;
  attribute_store_set_reported(granted_keys_node,
                               &granted_keys,
                               sizeof(granted_keys));

  // Inclusion Protocol does not exist yet:
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                    attribute_store_get_first_child_by_type(
                      node_id_node,
                      ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL));

  granted_keys = ZWAVE_CONTROLLER_S2_ACCESS_KEY;
  attribute_store_set_reported(granted_keys_node,
                               &granted_keys,
                               sizeof(granted_keys));

  // Inclusion Protocol does not exist still:
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                    attribute_store_get_first_child_by_type(
                      node_id_node,
                      ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL));

  // Now set it to 0:
  granted_keys = 0;
  attribute_store_set_reported(granted_keys_node,
                               &granted_keys,
                               sizeof(granted_keys));

  attribute_store_node_t inclusion_protocol_node
    = attribute_store_get_first_child_by_type(
      node_id_node,
      ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, inclusion_protocol_node);

  zwave_protocol_t protocol = 0xFF;
  attribute_store_get_reported(inclusion_protocol_node,
                               &protocol,
                               sizeof(protocol));
  TEST_ASSERT_EQUAL(PROTOCOL_ZWAVE, protocol);
}
