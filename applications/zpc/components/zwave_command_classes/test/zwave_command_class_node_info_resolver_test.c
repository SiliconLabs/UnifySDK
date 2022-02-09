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
//Generic includes
#include "unity.h"
#include <string.h>

// Component being tested
#include "zwave_command_class_node_info_resolver.h"
#include "zwave_command_class_node_info_resolver.c"

// Mock includes
#include "zwave_controller_utils_mock.h"
#include "zwave_controller_keyset_mock.h"
#include "zwave_utils_mock.h"
#include "zwave_controller_callbacks_mock.h"
#include "zwave_controller_keyset_mock.h"
#include "attribute_store_mock.h"
#include "zpc_attribute_store_mock.h"
#include "zpc_attribute_store_network_helper_mock.h"
#include "attribute_resolver_mock.h"
#include "attribute_store_helper_mock.h"
#include "zwave_unid_mock.h"
#include "zwave_utils_mock.h"
#include "zwave_security_validation_mock.h"

// Static test variables
static unid_t test_unid = "zw-38427";
static attribute_store_node_t test_endpoint_id_node;
static attribute_store_node_t test_node_id_node;
static zwave_keyset_t test_supporting_node_keys;
static zwave_controller_encapsulation_scheme_t test_supporting_node_scheme;
static zwave_endpoint_id_t test_endpoint_id;
static zwave_node_id_t test_node_id;

static uint8_t received_frame[ZWAVE_MAX_FRAME_SIZE];
static uint16_t received_frame_length;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Called before each and every test
void setUp()
{
  memset(received_frame, 0, sizeof(received_frame));
  received_frame_length = 0;
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

static void create_secure_nifs_if_missing_happy_case_verification(
  attribute_store_node_t node_id_node)
{
  test_node_id = 34;
  attribute_store_get_reported_ExpectAndReturn(node_id_node,
                                               NULL,
                                               sizeof(zwave_node_id_t),
                                               SL_STATUS_OK);
  attribute_store_get_reported_IgnoreArg_value();
  attribute_store_get_reported_ReturnMemThruPtr_value(&test_node_id,
                                                      sizeof(test_node_id));

  get_zpc_node_id_node_ExpectAndReturn(2);

  test_supporting_node_keys = 0x34;
  zwave_get_node_granted_keys_ExpectAndReturn(test_node_id, NULL, SL_STATUS_OK);
  zwave_get_node_granted_keys_IgnoreArg_keys();
  zwave_get_node_granted_keys_ReturnThruPtr_keys(&test_supporting_node_keys);

  test_supporting_node_scheme = 38;
  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    test_supporting_node_keys,
    test_supporting_node_scheme);

  zwave_controller_encapsulation_scheme_greater_equal_ExpectAndReturn(
    test_supporting_node_scheme,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0,
    true);

  test_endpoint_id_node = 38;
  attribute_store_get_node_child_by_type_ExpectAndReturn(node_id_node,
                                                         ATTRIBUTE_ENDPOINT_ID,
                                                         0,
                                                         test_endpoint_id_node);
  attribute_store_add_if_missing_Expect(test_endpoint_id_node, NULL, 1);
  attribute_store_add_if_missing_IgnoreArg_attributes();

  // No more enpoints
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    node_id_node,
    ATTRIBUTE_ENDPOINT_ID,
    1,
    ATTRIBUTE_STORE_INVALID_NODE);
}

void test_zwave_command_class_node_info_resolver_init()
{
  // We expect to know which functions/variables are
  // registered since we include the .c file directly
  zwave_controller_register_callbacks_ExpectAndReturn(
    &zwave_controller_callbacks,
    SL_STATUS_OK);

  attribute_resolver_register_rule_ExpectAndReturn(ATTRIBUTE_ZWAVE_NIF,
                                                   NULL,
                                                   &resolve_node_info,
                                                   SL_STATUS_OK);

  attribute_resolver_register_rule_ExpectAndReturn(ATTRIBUTE_ZWAVE_SECURE_NIF,
                                                   NULL,
                                                   &resolve_secure_node_info,
                                                   SL_STATUS_OK);

  attribute_store_register_callback_by_type_ExpectAndReturn(
    &on_non_secure_nif_update,
    ATTRIBUTE_ZWAVE_NIF,
    SL_STATUS_OK);

  attribute_store_register_callback_by_type_ExpectAndReturn(
    &on_granted_security_key_update,
    ATTRIBUTE_GRANTED_SECURITY_KEYS,
    SL_STATUS_OK);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_node_info_resolver_init());
}

void test_zwave_command_class_resolve_node_info_root_device()
{
  attribute_store_node_t test_updated_node = 0x87;
  const uint8_t expected_frame[]
    = {ZWAVE_PROTOCOL_COMMAND_CLASS, ZWAVE_NODE_INFO_REQUEST_COMMAND};

  attribute_store_get_node_type_ExpectAndReturn(test_updated_node,
                                                ATTRIBUTE_ZWAVE_NIF);

  attribute_store_node_t test_node_id_node = 23;
  attribute_store_get_first_parent_with_type_ExpectAndReturn(test_updated_node,
                                                             ATTRIBUTE_NODE_ID,
                                                             test_node_id_node);
  get_zpc_node_id_node_ExpectAndReturn(test_node_id_node + 1);

  // The function should find out the endpoint ID now
  test_endpoint_id = 0;
  attribute_store_network_helper_get_endpoint_id_from_node_ExpectAndReturn(
    test_updated_node,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_endpoint_id_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_endpoint_id_from_node_ReturnThruPtr_zwave_endpoint_id(
    &test_endpoint_id);

  // Call the function to test
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    resolve_node_info(test_updated_node,
                                      received_frame,
                                      &received_frame_length));

  // It should have returned the request NIF frame.
  TEST_ASSERT_EQUAL(sizeof(request_node_info_frame_t), received_frame_length);
  // We can only verify the payload of the command
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(request_node_info_frame_t));
}

void test_zwave_command_class_resolve_node_info_zpc_node()
{
  attribute_store_node_t test_updated_node = 0x87;
  attribute_store_get_node_type_ExpectAndReturn(test_updated_node,
                                                ATTRIBUTE_ZWAVE_NIF);

  attribute_store_node_t test_node_id_node = 23;
  attribute_store_get_first_parent_with_type_ExpectAndReturn(test_updated_node,
                                                             ATTRIBUTE_NODE_ID,
                                                             test_node_id_node);
  get_zpc_node_id_node_ExpectAndReturn(test_node_id_node);
  attribute_store_delete_node_ExpectAndReturn(test_updated_node, SL_STATUS_OK);

  // Call the function to test
  TEST_ASSERT_EQUAL(SL_STATUS_ALREADY_EXISTS,
                    resolve_node_info(test_updated_node,
                                      received_frame,
                                      &received_frame_length));

  TEST_ASSERT_EQUAL(0, received_frame_length);
}

void test_zwave_command_class_resolve_node_info_attribute_store_fails()
{
  attribute_store_node_t test_updated_node = 123;

  attribute_store_get_node_type_ExpectAndReturn(test_updated_node,
                                                ATTRIBUTE_ZWAVE_NIF);

  attribute_store_node_t test_node_id_node = 23;
  attribute_store_get_first_parent_with_type_ExpectAndReturn(test_updated_node,
                                                             ATTRIBUTE_NODE_ID,
                                                             test_node_id_node);
  get_zpc_node_id_node_ExpectAndReturn(test_node_id_node + 1);

  // The function should find out the endpoint ID now
  attribute_store_network_helper_get_endpoint_id_from_node_ExpectAndReturn(
    test_updated_node,
    NULL,
    SL_STATUS_FAIL);
  attribute_store_network_helper_get_endpoint_id_from_node_IgnoreArg_zwave_endpoint_id();

  // Call the function to test
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    resolve_node_info(test_updated_node,
                                      received_frame,
                                      &received_frame_length));

  // No frame should be returned
  TEST_ASSERT_EQUAL(0, received_frame_length);
}

static void zwave_command_class_multi_channel_capability_get_verification(
  attribute_store_node_t node, zwave_endpoint_id_t endpoint)
{
  attribute_store_node_t endpoint_node = 3847;
  attribute_store_get_first_parent_with_type_ExpectAndReturn(
    node,
    ATTRIBUTE_ENDPOINT_ID,
    endpoint_node);

  test_endpoint_id = endpoint;
  attribute_store_read_value_ExpectAndReturn(endpoint_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(zwave_endpoint_id_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &test_endpoint_id,
    sizeof(test_endpoint_id));
}

void test_zwave_command_class_resolve_node_info_endpoint()
{
  attribute_store_node_t test_updated_node = 0x891A;

  attribute_store_get_node_type_ExpectAndReturn(test_updated_node,
                                                ATTRIBUTE_ZWAVE_NIF);

  attribute_store_node_t test_node_id_node = 23;
  attribute_store_get_first_parent_with_type_ExpectAndReturn(test_updated_node,
                                                             ATTRIBUTE_NODE_ID,
                                                             test_node_id_node);
  get_zpc_node_id_node_ExpectAndReturn(test_node_id_node + 1);

  // The function should find out the endpoint ID now (returns >0)
  test_endpoint_id = 45;
  attribute_store_network_helper_get_endpoint_id_from_node_ExpectAndReturn(
    test_updated_node,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_endpoint_id_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_endpoint_id_from_node_ReturnThruPtr_zwave_endpoint_id(
    &test_endpoint_id);

  zwave_command_class_multi_channel_capability_get_verification(
    test_updated_node,
    test_endpoint_id);

  // Call the function to test
  sl_status_t received_status = resolve_node_info(test_updated_node,
                                                  received_frame,
                                                  &received_frame_length);

  if (test_endpoint_id == 0) {
    TEST_ASSERT_EQUAL(0, received_frame_length);
    TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED, received_status);
  } else {
    const uint8_t expected_frame[] = {COMMAND_CLASS_MULTI_CHANNEL_V4,
                                      MULTI_CHANNEL_CAPABILITY_GET_V4,
                                      test_endpoint_id};
    TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_length);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                  received_frame,
                                  received_frame_length);
    TEST_ASSERT_EQUAL(SL_STATUS_OK, received_status);
  }
}

void test_zwave_command_class_resolve_secure_node_info_attribute_store_fails()
{
  attribute_store_node_t test_updated_node = 0x2;

  attribute_store_get_node_type_ExpectAndReturn(test_updated_node,
                                                ATTRIBUTE_ZWAVE_SECURE_NIF);

  // The function should find out the Z-Wave NodeID
  attribute_store_network_helper_get_node_id_from_node_ExpectAndReturn(
    test_updated_node,
    NULL,
    SL_STATUS_FAIL);
  attribute_store_network_helper_get_node_id_from_node_IgnoreArg_zwave_node_id();

  // Call the function to test
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    resolve_secure_node_info(test_updated_node,
                                             received_frame,
                                             &received_frame_length));

  // Verify the received frame data
  TEST_ASSERT_EQUAL(0, received_frame_length);
}

void test_zwave_command_class_resolve_secure_node_info_s2_happy_case()
{
  attribute_store_node_t test_updated_node = 0x2;

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_SECURITY_2, SECURITY_2_COMMANDS_SUPPORTED_GET};

  attribute_store_get_node_type_ExpectAndReturn(test_updated_node,
                                                ATTRIBUTE_ZWAVE_SECURE_NIF);

  // The function should find out the Z-Wave NodeID
  test_node_id = 3;
  attribute_store_network_helper_get_node_id_from_node_ExpectAndReturn(
    test_updated_node,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_node_id_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_node_id_from_node_ReturnThruPtr_zwave_node_id(
    &test_node_id);

  test_supporting_node_keys = 0x12;
  zwave_get_node_granted_keys_ExpectAndReturn(test_node_id, NULL, SL_STATUS_OK);
  zwave_get_node_granted_keys_IgnoreArg_keys();
  zwave_get_node_granted_keys_ReturnThruPtr_keys(&test_supporting_node_keys);

  test_supporting_node_scheme
    = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED;
  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    test_supporting_node_keys,
    test_supporting_node_scheme);

  zwave_controller_encapsulation_scheme_greater_equal_ExpectAndReturn(
    test_supporting_node_scheme,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED,
    true);

  // Call the function to test
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    resolve_secure_node_info(test_updated_node,
                                             received_frame,
                                             &received_frame_length));

  // Verify the received frame data
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_zwave_command_class_resolve_secure_node_info_s0_happy_case()
{
  attribute_store_node_t test_updated_node = 0x2B;

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_SECURITY, SECURITY_COMMANDS_SUPPORTED_GET};

  attribute_store_get_node_type_ExpectAndReturn(test_updated_node,
                                                ATTRIBUTE_ZWAVE_SECURE_NIF);

  // The function should find out the Z-Wave NodeID
  test_node_id = 3;
  attribute_store_network_helper_get_node_id_from_node_ExpectAndReturn(
    test_updated_node,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_node_id_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_node_id_from_node_ReturnThruPtr_zwave_node_id(
    &test_node_id);

  test_supporting_node_keys = 0x80;
  zwave_get_node_granted_keys_ExpectAndReturn(test_node_id, NULL, SL_STATUS_OK);
  zwave_get_node_granted_keys_IgnoreArg_keys();
  zwave_get_node_granted_keys_ReturnThruPtr_keys(&test_supporting_node_keys);

  test_supporting_node_scheme = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0;
  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    test_supporting_node_keys,
    test_supporting_node_scheme);

  zwave_controller_encapsulation_scheme_greater_equal_ExpectAndReturn(
    test_supporting_node_scheme,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED,
    false);

  // Call the function to test
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    resolve_secure_node_info(test_updated_node,
                                             received_frame,
                                             &received_frame_length));

  // Verify the received frame data
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_zwave_command_class_resolve_secure_node_info_no_granted_keys()
{
  attribute_store_node_t test_updated_node = 0xAA;
  // Trigger the assert in this test
  attribute_store_get_node_type_ExpectAndReturn(
    test_updated_node,
    ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE);

  // The function should find out the Z-Wave NodeID
  test_node_id = 3;
  attribute_store_network_helper_get_node_id_from_node_ExpectAndReturn(
    test_updated_node,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_node_id_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_node_id_from_node_ReturnThruPtr_zwave_node_id(
    &test_node_id);

  test_supporting_node_keys = 0x80;
  zwave_get_node_granted_keys_ExpectAndReturn(test_node_id, NULL, SL_STATUS_OK);
  zwave_get_node_granted_keys_IgnoreArg_keys();
  zwave_get_node_granted_keys_ReturnThruPtr_keys(&test_supporting_node_keys);

  test_supporting_node_scheme = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    test_supporting_node_keys,
    test_supporting_node_scheme);

  zwave_controller_encapsulation_scheme_greater_equal_ExpectAndReturn(
    test_supporting_node_scheme,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED,
    false);

  // Call the function to test
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    resolve_secure_node_info(test_updated_node,
                                             received_frame,
                                             &received_frame_length));

  // Verify the received frame data
  TEST_ASSERT_EQUAL(0, received_frame_length);
}

void test_create_secure_nifs_happy_case()
{
  create_secure_nifs_if_missing_happy_case_verification(34);

  // Trigger the test
  create_secure_nifs_if_missing(34);
}

void test_create_secure_nifs_no_security()
{
  test_node_id_node = 0xB59;
  test_node_id      = 92;
  attribute_store_get_reported_ExpectAndReturn(test_node_id_node,
                                               NULL,
                                               sizeof(zwave_node_id_t),
                                               SL_STATUS_OK);
  attribute_store_get_reported_IgnoreArg_value();
  attribute_store_get_reported_ReturnMemThruPtr_value(&test_node_id,
                                                      sizeof(test_node_id));

  get_zpc_node_id_node_ExpectAndReturn(2);

  test_supporting_node_keys = 0x43;
  zwave_get_node_granted_keys_ExpectAndReturn(test_node_id, NULL, SL_STATUS_OK);
  zwave_get_node_granted_keys_IgnoreArg_keys();
  zwave_get_node_granted_keys_ReturnThruPtr_keys(&test_supporting_node_keys);

  test_supporting_node_scheme = 0x83;
  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    test_supporting_node_keys,
    test_supporting_node_scheme);

  zwave_controller_encapsulation_scheme_greater_equal_ExpectAndReturn(
    test_supporting_node_scheme,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0,
    false);

  // Trigger the test
  create_secure_nifs_if_missing(test_node_id_node);
}

void test_create_secure_nifs_not_for_the_zpc()
{
  test_node_id_node = 0xB59;
  test_node_id      = 92;
  attribute_store_get_reported_ExpectAndReturn(test_node_id_node,
                                               NULL,
                                               sizeof(zwave_node_id_t),
                                               SL_STATUS_OK);
  attribute_store_get_reported_IgnoreArg_value();
  attribute_store_get_reported_ReturnMemThruPtr_value(&test_node_id,
                                                      sizeof(test_node_id));

  get_zpc_node_id_node_ExpectAndReturn(test_node_id_node);

  // Trigger the test
  create_secure_nifs_if_missing(test_node_id_node);
}

void test_on_node_information_update()
{
  uint8_t test_node_info[]
    = {0x5E, 0x86, 0x38, 0x8C, 0x83, 0xF3, 0xFD, 0xFE, 0x00, 0x94};
  uint8_t test_node_info_length                   = sizeof(test_node_info);
  test_endpoint_id_node                           = 0x0085;
  attribute_store_node_t test_non_secure_nif_node = 0x0086;
  zwave_node_id_t node_id                         = 34;
  const zwave_node_info_t test_node_info_struct;

  zwave_unid_from_node_id_Expect(node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    0,
    test_endpoint_id_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_id_node,
    ATTRIBUTE_ZWAVE_NIF,
    0,
    test_non_secure_nif_node);

  zwave_command_class_list_pack_Expect(&test_node_info_struct, NULL, NULL);
  zwave_command_class_list_pack_IgnoreArg_node_info();
  zwave_command_class_list_pack_IgnoreArg_nif();
  zwave_command_class_list_pack_IgnoreArg_nif_length();
  zwave_command_class_list_pack_ReturnArrayThruPtr_nif(test_node_info,
                                                       sizeof(test_node_info));
  zwave_command_class_list_pack_ReturnThruPtr_nif_length(
    &test_node_info_length);

  zwave_security_validation_is_s2_nif_downgrade_attack_detected_ExpectAndReturn(
    node_id,
    NULL,
    sizeof(test_node_info),
    false);
  zwave_security_validation_is_s2_nif_downgrade_attack_detected_IgnoreArg_nif();

  attribute_store_set_node_attribute_value_ExpectAndReturn(
    test_non_secure_nif_node,
    REPORTED_ATTRIBUTE,
    test_node_info,
    test_node_info_length,
    SL_STATUS_OK);

  zwave_node_supports_command_class_ExpectAndReturn(COMMAND_CLASS_SECURITY_2,
                                                    node_id,
                                                    0,
                                                    true);
  zwave_security_validation_set_node_as_s2_capable_ExpectAndReturn(
    node_id,
    SL_STATUS_OK);

  // trigger the test
  on_node_information_update(node_id, &test_node_info_struct);
}

void test_on_nif_attribute_update()
{
  attribute_store_node_t test_node = 672;

  // Test 1: Nothing happens on deletion.
  on_non_secure_nif_update(test_node, ATTRIBUTE_DELETED);

  // Test 2: Now test NIF created but undefined:
  attribute_store_get_node_type_ExpectAndReturn(
    test_node,
    ATTRIBUTE_ZWAVE_GENERIC_DEVICE_CLASS);  // just trigger the assert here
  attribute_store_is_value_defined_ExpectAndReturn(test_node,
                                                   REPORTED_ATTRIBUTE,
                                                   false);

  on_non_secure_nif_update(test_node, ATTRIBUTE_CREATED);

  // Test 3: Now the happy case
  attribute_store_get_node_type_ExpectAndReturn(test_node, ATTRIBUTE_ZWAVE_NIF);
  attribute_store_is_value_defined_ExpectAndReturn(test_node,
                                                   REPORTED_ATTRIBUTE,
                                                   true);

  attribute_store_node_t node_id_node = 321;
  attribute_store_get_first_parent_with_type_ExpectAndReturn(test_node,
                                                             ATTRIBUTE_NODE_ID,
                                                             node_id_node);

  create_secure_nifs_if_missing_happy_case_verification(node_id_node);
  on_non_secure_nif_update(test_node, ATTRIBUTE_UPDATED);
}
