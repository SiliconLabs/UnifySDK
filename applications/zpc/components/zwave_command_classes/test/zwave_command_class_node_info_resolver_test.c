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
// Component being tested
#include "zwave_command_class_node_info_resolver.h"

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
#include "ZW_classcmd.h"
#include "zwave_utils.h"
#include "zwave_controller_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "attribute_resolver_mock.h"
#include "zwave_controller_callbacks_mock.h"
#include "zwave_controller_keyset_mock.h"
#include "zwave_controller_utils_mock.h"
#include "zwave_controller_storage_mock.h"
#include "zwave_network_management_mock.h"

// Static variables
static attribute_resolver_function_t resolve_secure_node_info = NULL;
static attribute_resolver_function_t resolve_node_info        = NULL;
static attribute_resolver_callback_t on_nif_resolution_abort  = NULL;

static const zwave_controller_callbacks_t *controller_callbacks = NULL;

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
  if (node_type == ATTRIBUTE_ZWAVE_NIF) {
    resolve_node_info = get_func;
  } else if (node_type == ATTRIBUTE_ZWAVE_SECURE_NIF) {
    resolve_secure_node_info = get_func;
  } else {
    TEST_FAIL_MESSAGE("Attribute rule registration on a wrong type");
  }
  return SL_STATUS_OK;
}

void attribute_resolver_set_resolution_give_up_listener_stub(
  attribute_store_type_t node_type,
  attribute_resolver_callback_t callback,
  int cmock_num_calls)
{
  if (ATTRIBUTE_ZWAVE_NIF == node_type) {
    on_nif_resolution_abort = callback;
  }
}

sl_status_t zwave_controller_register_callbacks_stub(
  const zwave_controller_callbacks_t *callbacks, int cmock_num_calls)
{
  controller_callbacks = callbacks;
  TEST_ASSERT_NOT_NULL(controller_callbacks->on_node_information);
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
  zwave_controller_register_callbacks_Stub(
    &zwave_controller_register_callbacks_stub);

  // Rule registration
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // Give up listener registration
  attribute_resolver_set_resolution_give_up_listener_Stub(
    &attribute_resolver_set_resolution_give_up_listener_stub);

  resolve_secure_node_info = NULL;
  resolve_node_info        = NULL;
  on_nif_resolution_abort  = NULL;
  controller_callbacks     = NULL;
  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size = 0;

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_node_info_resolver_init());
}

/// Called before each and every test
void setUp()
{
  zwave_network_management_get_node_id_IgnoreAndReturn(zpc_node_id);
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_set_home_id(home_id);

  init_verification();
}

void test_resolve_node_info_endpoint_4()
{
  attribute_store_node_t nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);

  TEST_ASSERT_NOT_NULL(resolve_node_info);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    resolve_node_info(nif_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_MULTI_CHANNEL_V4,
                                    MULTI_CHANNEL_CAPABILITY_GET_V4,
                                    endpoint_id};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_resolve_node_info_endpoint_0()
{
  attribute_store_node_t nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);
  zwave_endpoint_id_t new_endpoint_id = 0;
  attribute_store_set_reported(endpoint_id_node,
                               &new_endpoint_id,
                               sizeof(new_endpoint_id));

  TEST_ASSERT_NOT_NULL(resolve_node_info);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    resolve_node_info(nif_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {0x01, 0x02};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_resolve_node_info_zpc()
{
  attribute_store_node_t nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, zpc_endpoint_id_node);

  TEST_ASSERT_NOT_NULL(resolve_node_info);
  TEST_ASSERT_EQUAL(
    SL_STATUS_ALREADY_EXISTS,
    resolve_node_info(nif_node, received_frame, &received_frame_size));

  TEST_ASSERT_FALSE(attribute_store_node_exists(nif_node));
}

void test_resolve_node_info_undefined_endpoint()
{
  attribute_store_node_t nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);
  attribute_store_undefine_reported(endpoint_id_node);

  TEST_ASSERT_NOT_NULL(resolve_node_info);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    resolve_node_info(nif_node, received_frame, &received_frame_size));

  TEST_ASSERT_EQUAL(0, received_frame_size);
}

void test_resolve_secure_node_info_s2_key()
{
  attribute_store_node_t secure_nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_SECURE_NIF, endpoint_id_node);

  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    0,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED);
  zwave_controller_encapsulation_scheme_greater_equal_ExpectAndReturn(
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED,
    true);

  TEST_ASSERT_NOT_NULL(resolve_secure_node_info);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    resolve_secure_node_info(secure_nif_node,
                                             received_frame,
                                             &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_SECURITY_2, SECURITY_2_COMMANDS_SUPPORTED_GET};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_resolve_secure_node_info_s0_key()
{
  attribute_store_node_t secure_nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_SECURE_NIF, endpoint_id_node);

  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    0,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0);
  zwave_controller_encapsulation_scheme_greater_equal_ExpectAndReturn(
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED,
    false);

  TEST_ASSERT_NOT_NULL(resolve_secure_node_info);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    resolve_secure_node_info(secure_nif_node,
                                             received_frame,
                                             &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_SECURITY, SECURITY_COMMANDS_SUPPORTED_GET};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_resolve_secure_node_info_no_granted_key()
{
  attribute_store_node_t secure_nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_SECURE_NIF, endpoint_id_node);

  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    0,
    ZWAVE_CONTROLLER_ENCAPSULATION_NONE);
  zwave_controller_encapsulation_scheme_greater_equal_ExpectAndReturn(
    ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED,
    false);

  TEST_ASSERT_NOT_NULL(resolve_secure_node_info);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    resolve_secure_node_info(secure_nif_node,
                                             received_frame,
                                             &received_frame_size));

  TEST_ASSERT_EQUAL(0, received_frame_size);
}

void test_creating_secure_nif_after_nif_granted_keys_updates()
{
  // Add a second endpoint with value 1:
  attribute_store_node_t endpoint_1_node
    = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, node_id_node);

  // Add a non-secure nif:
  attribute_store_node_t nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);

  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    0x00,
    ZWAVE_CONTROLLER_ENCAPSULATION_NONE);
  zwave_controller_encapsulation_scheme_greater_equal_ExpectAndReturn(
    ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0,
    false);

  const uint8_t nif_contents[] = {1, 2, 3, 4, 5, 6};
  attribute_store_set_reported(nif_node, nif_contents, sizeof(nif_contents));

  attribute_store_node_t granted_keys_node
    = attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS, node_id_node);

  zwave_keyset_t keys = 0x84;
  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    keys,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS);
  zwave_controller_encapsulation_scheme_greater_equal_ExpectAndReturn(
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0,
    true);

  attribute_store_set_reported(granted_keys_node, &keys, sizeof(keys));

  attribute_store_node_t secure_nif_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE_ZWAVE_SECURE_NIF);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, secure_nif_node);

  attribute_store_node_t secure_nif_1_node
    = attribute_store_get_first_child_by_type(endpoint_1_node,
                                              ATTRIBUTE_ZWAVE_SECURE_NIF);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, secure_nif_1_node);

  // Delete the granted keys / nifs for test coverage.
  attribute_store_delete_node(granted_keys_node);
  attribute_store_delete_node(nif_node);
}

void test_not_creating_secure_nif_for_zpc()
{
  // Add a non-secure nif:
  attribute_store_node_t nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, zpc_endpoint_id_node);

  const uint8_t nif_contents[] = {1, 2, 3, 4, 5, 6};
  attribute_store_set_reported(nif_node, nif_contents, sizeof(nif_contents));

  attribute_store_node_t granted_keys_node
    = attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS,
                               zpc_node_id_node);

  zwave_keyset_t keys = 3;
  attribute_store_set_reported(granted_keys_node, &keys, sizeof(keys));

  attribute_store_node_t secure_nif_node
    = attribute_store_get_first_child_by_type(zpc_endpoint_id_node,
                                              ATTRIBUTE_ZWAVE_SECURE_NIF);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, secure_nif_node);

  attribute_store_node_t secure_nif_1_node
    = attribute_store_get_first_child_by_type(zpc_endpoint_id_node,
                                              ATTRIBUTE_ZWAVE_SECURE_NIF);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, secure_nif_1_node);
}

void test_not_creating_secure_nif_for_valueless_node_id()
{
  attribute_store_undefine_reported(zpc_node_id_node);
  // Add a non-secure nif:
  attribute_store_node_t nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, zpc_endpoint_id_node);

  const uint8_t nif_contents[] = {1, 2, 3, 4, 5, 6};
  attribute_store_set_reported(nif_node, nif_contents, sizeof(nif_contents));

  attribute_store_node_t granted_keys_node
    = attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS,
                               zpc_node_id_node);

  zwave_keyset_t keys = 3;
  attribute_store_set_reported(granted_keys_node, &keys, sizeof(keys));

  attribute_store_node_t secure_nif_node
    = attribute_store_get_first_child_by_type(zpc_endpoint_id_node,
                                              ATTRIBUTE_ZWAVE_SECURE_NIF);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, secure_nif_node);

  attribute_store_node_t secure_nif_1_node
    = attribute_store_get_first_child_by_type(zpc_endpoint_id_node,
                                              ATTRIBUTE_ZWAVE_SECURE_NIF);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, secure_nif_1_node);
}

void test_on_node_information_update()
{
  TEST_ASSERT_NOT_NULL(controller_callbacks->on_node_information);

  const zwave_node_info_t node_info
    = {.listening_protocol        = 2,
       .optional_protocol         = 3,
       .basic_device_class        = 4,
       .generic_device_class      = 5,
       .specific_device_class     = 6,
       .command_class_list_length = 3,
       .command_class_list        = {0x9F, 0x98, 0x25}};

  uint8_t command_class_list_u8[] = {0x9F, 0x98, 0x25};
  uint8_t nif_length              = sizeof(command_class_list_u8);

  zwave_command_class_list_pack_Expect(&node_info, NULL, NULL);
  zwave_command_class_list_pack_IgnoreArg_nif();
  zwave_command_class_list_pack_IgnoreArg_nif_length();
  zwave_command_class_list_pack_ReturnArrayThruPtr_nif(
    command_class_list_u8,
    sizeof(command_class_list_u8));
  zwave_command_class_list_pack_ReturnArrayThruPtr_nif_length(
    &nif_length,
    sizeof(nif_length));

  zwave_controller_storage_is_node_s2_capable_ExpectAndReturn(node_id, true);
  is_command_class_in_supported_list_IgnoreAndReturn(true);
  controller_callbacks->on_node_information(node_id, &node_info);
}

void test_on_node_information_update_downgrade()
{
  TEST_ASSERT_NOT_NULL(controller_callbacks->on_node_information);

  const zwave_node_info_t node_info
    = {.listening_protocol        = 2,
       .optional_protocol         = 3,
       .basic_device_class        = 4,
       .generic_device_class      = 5,
       .specific_device_class     = 6,
       .command_class_list_length = 3,
       .command_class_list        = {0x9F, 0x98, 0x25}};

  uint8_t command_class_list_u8[] = {0x9F, 0x98, 0x25};
  uint8_t nif_length              = sizeof(command_class_list_u8);

  zwave_command_class_list_pack_Expect(&node_info, NULL, NULL);
  zwave_command_class_list_pack_IgnoreArg_nif();
  zwave_command_class_list_pack_IgnoreArg_nif_length();
  zwave_command_class_list_pack_ReturnArrayThruPtr_nif(
    command_class_list_u8,
    sizeof(command_class_list_u8));
  zwave_command_class_list_pack_ReturnArrayThruPtr_nif_length(
    &nif_length,
    sizeof(nif_length));

  zwave_controller_storage_is_node_s2_capable_ExpectAndReturn(node_id, true);
  is_command_class_in_supported_list_IgnoreAndReturn(false);
  controller_callbacks->on_node_information(node_id, &node_info);
}

void test_on_nif_resolution_aborted()
{
  TEST_ASSERT_NOT_NULL(on_nif_resolution_abort);

  // Add 2 NIFs, one under endpoint 0 and one under endpoint 2
  zwave_endpoint_id_t endpoint_id_value = 0;
  attribute_store_node_t endpoint_0_node
    = attribute_store_emplace(node_id_node,
                              ATTRIBUTE_ENDPOINT_ID,
                              &endpoint_id_value,
                              sizeof(endpoint_id_value));
  attribute_store_node_t nif_0_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_0_node);
  endpoint_id_value = 2;
  attribute_store_node_t endpoint_2_node
    = attribute_store_emplace(node_id_node,
                              ATTRIBUTE_ENDPOINT_ID,
                              &endpoint_id_value,
                              sizeof(endpoint_id_value));
  attribute_store_node_t nif_2_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_2_node);

  // Now check what happens when endpoint 0 NIF cannot be resolved:
  on_nif_resolution_abort(nif_0_node);
  TEST_ASSERT_EQUAL(true, attribute_store_node_exists(nif_0_node));
  on_nif_resolution_abort(nif_2_node);
  TEST_ASSERT_EQUAL(false, attribute_store_node_exists(nif_2_node));
}