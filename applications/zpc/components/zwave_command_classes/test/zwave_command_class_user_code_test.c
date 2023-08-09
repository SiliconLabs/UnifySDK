/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "zwave_command_class_user_code.h"

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"

// Interface includes
#include "attribute_store_defined_attribute_types.h"
#include "zwave_command_class_generic_types.h"

// ZPC includes
#include "zpc_attribute_store_type_registration.h"
#include "zwave_utils.h"
#include "zwave_controller_types.h"

// Generic includes
#include <string.h>

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "attribute_resolver_mock.h"
#include "zpc_attribute_resolver_mock.h"
#include "zwave_command_handler_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_USER_CODE_##type

// Static test variables
static attribute_resolver_function_t capabilities_get = NULL;
static attribute_resolver_function_t checksum_get     = NULL;
static attribute_resolver_function_t delete_all_codes = NULL;
static attribute_resolver_function_t keypad_mode_get  = NULL;
static attribute_resolver_function_t keypad_mode_set  = NULL;
static attribute_resolver_function_t admin_code_get   = NULL;
static attribute_resolver_function_t admin_code_set   = NULL;
static attribute_resolver_function_t user_number_get  = NULL;
static attribute_resolver_function_t user_code_get    = NULL;
static attribute_resolver_function_t user_code_set    = NULL;
static zpc_resolver_event_notification_function_t
  on_delete_all_send_data_complete
  = NULL;
static zwave_command_handler_t handler = {};
static zwave_controller_connection_info_t info = {};
static uint8_t received_frame[255]     = {};
static uint16_t received_frame_size    = 0;

#define KEY_BITMASK_SIZE 16
static const uint8_t V1_SUPPORTED_KEYS[KEY_BITMASK_SIZE] = {0x00,
                                                            0x00,
                                                            0x00,
                                                            0x00,
                                                            0x00,
                                                            0x00,
                                                            0xFF,
                                                            0x03,
                                                            0x00,
                                                            0x00,
                                                            0x00,
                                                            0x00,
                                                            0x00,
                                                            0x00,
                                                            0x00,
                                                            0x00};

// Stub functions
static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  if (node_type == ATTRIBUTE(NUMBER_OF_USERS)) {
    TEST_ASSERT_NULL(set_func);
    user_number_get = get_func;
  } else if (node_type == ATTRIBUTE(DELETE_ALL_REQUESTED)) {
    TEST_ASSERT_NULL(get_func);
    delete_all_codes = set_func;
  } else if (node_type == ATTRIBUTE(CODE)
             || node_type == ATTRIBUTE(USER_ID_STATUS)) {
    user_code_set = set_func;
    user_code_get = get_func;
  } else if (node_type == ATTRIBUTE(CHECKSUM)) {
    TEST_ASSERT_NULL(set_func);
    checksum_get = get_func;
  } else if (node_type == ATTRIBUTE(ADMIN_CODE)) {
    admin_code_set = set_func;
    admin_code_get = get_func;
  } else if (node_type == ATTRIBUTE(KEYPAD_MODE)) {
    keypad_mode_set = set_func;
    keypad_mode_get = get_func;
  } else if (node_type == ATTRIBUTE(SUPPORTED_FLAGS)) {
    TEST_ASSERT_NULL(set_func);
    capabilities_get = get_func;
  } else {
    TEST_FAIL_MESSAGE("Attribute rule registration on an unexpected type");
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t command_class_handler, int cmock_num_calls)
{
  handler = command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_USER_CODE_V2, handler.command_class);
  TEST_ASSERT_EQUAL(USER_CODE_VERSION_V2, handler.version);
  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_NULL(handler.support_handler);
  TEST_ASSERT_FALSE(handler.manual_security_validation);

  return SL_STATUS_OK;
}

static sl_status_t register_send_event_handler_stub(
  attribute_store_type_t type,
  zpc_resolver_event_notification_function_t function,
  int cmock_num_calls)
{
  if (ATTRIBUTE(DELETE_ALL_REQUESTED) == type) {
    on_delete_all_send_data_complete = function;
  }
  TEST_ASSERT_NOT_NULL(on_delete_all_send_data_complete);

  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zpc_attribute_store_register_known_attribute_types();
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
  capabilities_get                 = NULL;
  checksum_get                     = NULL;
  delete_all_codes                 = NULL;
  keypad_mode_get                  = NULL;
  keypad_mode_set                  = NULL;
  admin_code_get                   = NULL;
  admin_code_set                   = NULL;
  user_number_get                  = NULL;
  user_code_get                    = NULL;
  user_code_set                    = NULL;
  admin_code_set                   = NULL;
  admin_code_get                   = NULL;
  on_delete_all_send_data_complete = NULL;

  memset(&handler, 0, sizeof(zwave_command_handler_t));
  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size = 0;

  // Prep our attribute store
  zpc_attribute_store_test_helper_create_network();

  // Prepare a default connection info with the supporting node
  memset(&info, 0, sizeof(zwave_controller_connection_info_t));
  attribute_store_get_reported(node_id_node,
                               &(info.remote.node_id),
                               sizeof(info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(info.remote.endpoint_id),
                               sizeof(info.remote.endpoint_id));

  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // On send complete handler
  register_send_event_handler_Stub(&register_send_event_handler_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_user_code_init());

  // Check all the handlers have been intercepted correctly:
  TEST_ASSERT_NOT_NULL(keypad_mode_get);
  TEST_ASSERT_NOT_NULL(keypad_mode_set);
  TEST_ASSERT_NOT_NULL(capabilities_get);
  TEST_ASSERT_NOT_NULL(checksum_get);
  TEST_ASSERT_NOT_NULL(delete_all_codes);
  TEST_ASSERT_NOT_NULL(admin_code_get);
  TEST_ASSERT_NOT_NULL(admin_code_set);
  TEST_ASSERT_NOT_NULL(user_number_get);
  TEST_ASSERT_NOT_NULL(user_code_get);
  TEST_ASSERT_NOT_NULL(user_code_set);
  TEST_ASSERT_NOT_NULL(admin_code_set);
  TEST_ASSERT_NOT_NULL(admin_code_get);
  TEST_ASSERT_NOT_NULL(on_delete_all_send_data_complete);
}

/// Called after each and every test
void tearDown() {}

void test_zwave_command_class_user_code_number_get()
{
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    user_number_get(ATTRIBUTE_STORE_INVALID_NODE,
                                    received_frame,
                                    &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, USERS_NUMBER_GET_V2};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

static attribute_store_node_t create_user_code_nodes(uint16_t user_id)
{
  attribute_store_node_t data_node
    = attribute_store_create_child_if_missing(endpoint_id_node,
                                              ATTRIBUTE(DATA));

  return attribute_store_emplace(data_node,
                                 ATTRIBUTE(USER_ID),
                                 &user_id,
                                 sizeof(user_id));
}

static void receive_ctt_v2_capabilities()
{
  // Example capabilities report from CTT
  const uint8_t capabilities_report_frame[] = {0x63,
                                               0x07,
                                               0xC1,
                                               0x17,
                                               0xA1,
                                               0x07,
                                               0x0C,
                                               0x00,
                                               0x00,
                                               0x00,
                                               0x00,
                                               0x00,
                                               0x00,
                                               0xFF,
                                               0x03,
                                               0xFE,
                                               0xFF,
                                               0xFF,
                                               0x07};
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            capabilities_report_frame,
                                            sizeof(capabilities_report_frame)));
}

void test_zwave_command_class_user_code_get_unknown_version()
{
  attribute_store_node_t user_id_node = create_user_code_nodes(3);
  attribute_store_node_t user_code_node
    = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));

  TEST_ASSERT_EQUAL(
    SL_STATUS_IS_WAITING,
    user_code_get(user_code_node, received_frame, &received_frame_size));
  TEST_ASSERT_EQUAL(0, received_frame_size);
}

void test_zwave_command_class_user_code_get_v1_happy_case()
{
  // clang-format off
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t user_id_node = create_user_code_nodes(3);
  attribute_store_node_t user_code_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    user_code_get(user_code_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, USER_CODE_GET_V2, 3};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_user_code_get_v1_too_high_user_id()
{
  // clang-format off
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t user_id_node = create_user_code_nodes(256);
  attribute_store_node_t user_code_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_ALREADY_EXISTS,
    user_code_get(user_code_node, received_frame, &received_frame_size));
  TEST_ASSERT_EQUAL(0, received_frame_size);
  TEST_ASSERT_FALSE(attribute_store_node_exists(user_code_node));
}

void test_zwave_command_class_user_code_get_v2_low_number()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t user_id_node = create_user_code_nodes(55);
  attribute_store_node_t user_code_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    user_code_get(user_code_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, EXTENDED_USER_CODE_GET_V2, 0, 55, 0};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_user_code_get_v2_high_number()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t user_id_node = create_user_code_nodes(257);
  attribute_store_node_t user_code_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    user_code_get(user_code_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, EXTENDED_USER_CODE_GET_V2, 1, 1, 0};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_user_code_capabilities_get_unknown_version()
{
  // clang-format off
  const zwave_cc_version_t version = 0;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t capabilities_node = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(CAPABILITIES));
  attribute_store_node_t flags_node = attribute_store_get_first_child_by_type(capabilities_node, ATTRIBUTE(SUPPORTED_FLAGS));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_IS_WAITING,
    capabilities_get(flags_node, received_frame, &received_frame_size));

  TEST_ASSERT_EQUAL(0, received_frame_size);
}

void test_zwave_command_class_user_code_capabilities_get_v1()
{
  // clang-format off
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t capabilities_node = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(CAPABILITIES));
  attribute_store_node_t flags_node = attribute_store_get_first_child_by_type(capabilities_node, ATTRIBUTE(SUPPORTED_FLAGS));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_ALREADY_EXISTS,
    capabilities_get(flags_node, received_frame, &received_frame_size));
  TEST_ASSERT_EQUAL(0, received_frame_size);

  const uint8_t EXPECTED_FLAGS = 0x00;
  uint8_t actual_flags         = 0xFF;
  attribute_store_get_reported(flags_node,
                               &actual_flags,
                               sizeof(EXPECTED_FLAGS));
  TEST_ASSERT_EQUAL(EXPECTED_FLAGS, actual_flags);
}

void test_zwave_command_class_user_code_capabilities_get_v2()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t capabilities_node = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(CAPABILITIES));
  attribute_store_node_t flags_node = attribute_store_get_first_child_by_type(capabilities_node, ATTRIBUTE(SUPPORTED_FLAGS));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    capabilities_get(flags_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, USER_CODE_CAPABILITIES_GET_V2};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_user_code_checksum_get_unknown_version()
{
  // clang-format off
  const zwave_cc_version_t version = 0;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t checksum_node = attribute_store_create_child_if_missing(data_node, ATTRIBUTE(CHECKSUM));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_IS_WAITING,
    checksum_get(checksum_node, received_frame, &received_frame_size));
  TEST_ASSERT_EQUAL(0, received_frame_size);
}

void test_zwave_command_class_user_code_checksum_get_v1()
{
  // clang-format off
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t checksum_node = attribute_store_create_child_if_missing(data_node, ATTRIBUTE(CHECKSUM));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_ALREADY_EXISTS,
    checksum_get(checksum_node, received_frame, &received_frame_size));
  TEST_ASSERT_EQUAL(0, received_frame_size);
  TEST_ASSERT_FALSE(attribute_store_node_exists(checksum_node));
}

void test_zwave_command_class_user_code_checksum_get_v2()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t checksum_node = attribute_store_create_child_if_missing(data_node, ATTRIBUTE(CHECKSUM));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    checksum_get(checksum_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, USER_CODE_CHECKSUM_GET_V2};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_admin_code_get_unknown_version()
{
  // clang-format off
  const zwave_cc_version_t version = 0;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t admin_code_node = attribute_store_create_child_if_missing(data_node, ATTRIBUTE(ADMIN_CODE));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_IS_WAITING,
    admin_code_get(admin_code_node, received_frame, &received_frame_size));
  TEST_ASSERT_EQUAL(0, received_frame_size);
}

void test_zwave_command_class_admin_code_get_v1()
{
  // clang-format off
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t admin_code_node = attribute_store_create_child_if_missing(data_node, ATTRIBUTE(ADMIN_CODE));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_ALREADY_EXISTS,
    admin_code_get(admin_code_node, received_frame, &received_frame_size));
  TEST_ASSERT_EQUAL(0, received_frame_size);
  TEST_ASSERT_FALSE(attribute_store_node_exists(admin_code_node));
}

void test_zwave_command_class_admin_code_get_v2()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t admin_code_node = attribute_store_create_child_if_missing(data_node, ATTRIBUTE(ADMIN_CODE));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    admin_code_get(admin_code_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_USER_CODE_V2, 0x0F};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_keypad_mode_get()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t keypad_mode_node = attribute_store_create_child_if_missing(data_node, ATTRIBUTE(KEYPAD_MODE));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    keypad_mode_get(keypad_mode_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, USER_CODE_KEYPAD_MODE_GET_V2};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_keypad_mode_set()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t keypad_mode_node = attribute_store_create_child_if_missing(data_node, ATTRIBUTE(KEYPAD_MODE));
  uint8_t keypad_mode = 18;
  attribute_store_set_reported(keypad_mode_node,&keypad_mode,sizeof(keypad_mode));
  // clang-format on

  // try with reported value only:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    keypad_mode_set(keypad_mode_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, USER_CODE_KEYPAD_MODE_SET_V2, keypad_mode};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);

  // try with desired value set:
  keypad_mode = 3;
  attribute_store_set_desired(keypad_mode_node,
                              &keypad_mode,
                              sizeof(keypad_mode));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    keypad_mode_set(keypad_mode_node, received_frame, &received_frame_size));

  const uint8_t expected_frame_2[]
    = {COMMAND_CLASS_USER_CODE_V2, USER_CODE_KEYPAD_MODE_SET_V2, keypad_mode};
  TEST_ASSERT_EQUAL(sizeof(expected_frame_2), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame_2,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_user_code_set_version_unknown()
{
  // clang-format off
  const zwave_cc_version_t version = 0;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t user_id_node = create_user_code_nodes(2);
  attribute_store_node_t user_code_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));
  const char desired_code[] = "53408";
  attribute_store_set_desired(user_code_node,desired_code,sizeof(desired_code));
  uint8_t user_id_status = 3;
  attribute_store_set_child_desired(user_id_node, ATTRIBUTE(USER_ID_STATUS), &user_id_status, sizeof(user_id_status));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_IS_WAITING,
    user_code_set(user_code_node, received_frame, &received_frame_size));
}

void test_zwave_command_class_user_code_set_v1()
{
  // clang-format off
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t user_id_node = create_user_code_nodes(6);
  attribute_store_node_t user_code_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));
  const char desired_code[] = "53408";
  attribute_store_set_desired(user_code_node,desired_code,sizeof(desired_code));
  uint8_t user_id_status = 3;
  attribute_store_set_child_desired(user_id_node, ATTRIBUTE(USER_ID_STATUS), &user_id_status, sizeof(user_id_status));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    user_code_set(user_code_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_USER_CODE_V2,
                                    USER_CODE_SET_V2,
                                    6,
                                    3,
                                    0x35,
                                    0x33,
                                    0x34,
                                    0x30,
                                    0x38};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_user_code_set_v1_too_high_user_id()
{
  // clang-format off
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t user_id_node = create_user_code_nodes(300);
  attribute_store_node_t user_code_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));
  const char desired_code[] = "90548A";
  attribute_store_set_desired(user_code_node,desired_code,sizeof(desired_code));
  uint8_t user_id_status = 3;
  attribute_store_set_child_desired(user_id_node, ATTRIBUTE(USER_ID_STATUS), &user_id_status, sizeof(user_id_status));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_ALREADY_EXISTS,
    user_code_set(user_code_node, received_frame, &received_frame_size));

  TEST_ASSERT_FALSE(attribute_store_node_exists(user_id_node));
  TEST_ASSERT_FALSE(attribute_store_node_exists(user_code_node));
}

void test_zwave_command_class_user_code_set_v1_erase_code()
{
  // clang-format off
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t user_id_node = create_user_code_nodes(66);
  attribute_store_node_t user_code_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));
  const char desired_code[] = "90548A";
  attribute_store_set_desired(user_code_node,desired_code,sizeof(desired_code));
  uint8_t user_id_status = 0;
  attribute_store_set_child_desired(user_id_node, ATTRIBUTE(USER_ID_STATUS), &user_id_status, sizeof(user_id_status));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    user_code_set(user_code_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_USER_CODE_V2,
                                    USER_CODE_SET_V2,
                                    66,
                                    0,
                                    0x00,
                                    0x00,
                                    0x00,
                                    0x00};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_user_code_set_v1_illegal_keys()
{
  // clang-format off
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t user_id_node = create_user_code_nodes(66);
  attribute_store_node_t user_code_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));
  const char desired_code[] = "90548A";
  attribute_store_set_desired(user_code_node,desired_code,sizeof(desired_code));
  uint8_t user_id_status = 0;
  attribute_store_set_child_desired(user_id_node, ATTRIBUTE(USER_ID_STATUS), &user_id_status, sizeof(user_id_status));
  attribute_store_node_t capabilities_node = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(CAPABILITIES));
  attribute_store_node_t flags_node = attribute_store_get_first_child_by_type(capabilities_node, ATTRIBUTE(SUPPORTED_FLAGS));
  // clang-format on

  // Get the default V1 capabilities filled up by requesting a capabilties get:
  TEST_ASSERT_EQUAL(
    SL_STATUS_ALREADY_EXISTS,
    capabilities_get(flags_node, received_frame, &received_frame_size));

  // Now try to set a pin code with a "A" in it.
  TEST_ASSERT_EQUAL(
    SL_STATUS_ALREADY_EXISTS,
    user_code_set(user_code_node, received_frame, &received_frame_size));

  TEST_ASSERT_EQUAL(0, received_frame_size);
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(user_code_node));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(user_id_node));
}

void test_zwave_command_class_user_code_set_v2()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  //attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t user_id_node = create_user_code_nodes(13320);
  attribute_store_node_t user_code_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));
  const char desired_code[] = "9B54C.8A";
  attribute_store_set_desired(user_code_node, desired_code, sizeof(desired_code));
  uint8_t user_id_status = 1;
  attribute_store_set_child_desired(user_id_node, ATTRIBUTE(USER_ID_STATUS), &user_id_status, sizeof(user_id_status));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    user_code_set(user_code_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {
    COMMAND_CLASS_USER_CODE_V2,
    EXTENDED_USER_CODE_SET_V2,
    1,
    0x34,  // User ID MSB
    0x08,  // User ID LSB
    1,
    8,
    0x39,  // ASCII 9
    0x42,  // ASCII B
    0x35,  // ASCII 5
    0x34,  // ASCII 4
    0x43,  // ASCII C
    0x2E,  // ASCII .
    0x38,  // ASCII 8
    0x41   // ASCII A
  };
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_user_code_set_v2_erase_code()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  //attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t user_id_node = create_user_code_nodes(13320);
  attribute_store_node_t user_code_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));
  const char desired_code[] = "9B54C.8A";
  attribute_store_set_desired(user_code_node, desired_code, sizeof(desired_code));
  uint8_t user_id_status = 0;
  attribute_store_set_child_desired(user_id_node, ATTRIBUTE(USER_ID_STATUS), &user_id_status, sizeof(user_id_status));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    user_code_set(user_code_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_USER_CODE_V2,
                                    EXTENDED_USER_CODE_SET_V2,
                                    1,
                                    0x34,  // User ID MSB
                                    0x08,  // User ID LSB
                                    0,
                                    0};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_user_code_admin_code_set()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t admin_code_node = attribute_store_create_child_if_missing(data_node, ATTRIBUTE(ADMIN_CODE));
  const char desired_code[] = "83945";
  attribute_store_set_desired(admin_code_node, desired_code, sizeof(desired_code));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    admin_code_set(admin_code_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, 0x0E, 5, 0x38, 0x33, 0x39, 0x34, 0x35};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_user_code_admin_code_set_unsupported_pin()
{
  // clang-format off
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t admin_code_node = attribute_store_create_child_if_missing(data_node, ATTRIBUTE(ADMIN_CODE));
  const char desired_code[] = "83945/";
  attribute_store_set_desired(admin_code_node, desired_code, sizeof(desired_code));
  attribute_store_node_t capabilities_node = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(CAPABILITIES));
  attribute_store_node_t flags_node = attribute_store_get_first_child_by_type(capabilities_node, ATTRIBUTE(SUPPORTED_FLAGS));
  // clang-format on

  // Get the default V1 capabilities filled up by requesting a capabilties get:
  TEST_ASSERT_EQUAL(
    SL_STATUS_ALREADY_EXISTS,
    capabilities_get(flags_node, received_frame, &received_frame_size));

  TEST_ASSERT_EQUAL(
    SL_STATUS_ALREADY_EXISTS,
    admin_code_set(admin_code_node, received_frame, &received_frame_size));

  TEST_ASSERT_EQUAL(0, received_frame_size);
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(admin_code_node));
}

void test_zwave_command_class_user_code_delete_all_unknown_version()
{
  // clang-format off
  const zwave_cc_version_t version = 0;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t delete_all_node = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(DELETE_ALL_REQUESTED));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_IS_WAITING,
    delete_all_codes(delete_all_node, received_frame, &received_frame_size));

  TEST_ASSERT_EQUAL(0, received_frame_size);
}

void test_zwave_command_class_user_code_delete_all_v1()
{
  // clang-format off
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t delete_all_node = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(DELETE_ALL_REQUESTED));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    delete_all_codes(delete_all_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_USER_CODE_V2,
                                    USER_CODE_SET_V2,
                                    0,
                                    0,
                                    0x00,
                                    0x00,
                                    0x00,
                                    0x00};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_user_code_delete_all_v2()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t delete_all_node = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(DELETE_ALL_REQUESTED));
  // clang-format on

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    delete_all_codes(delete_all_node, received_frame, &received_frame_size));

  /*
  printf("\n Received frame: [");
  for (int i = 0; i < received_frame_size; i++) {
    printf("0x%02X ", received_frame[i]);
  }
  printf("]\n");
  */

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, EXTENDED_USER_CODE_SET_V2, 1, 0, 0, 0, 0};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_user_code_on_delete_all_complete_invalid_rule()
{
  // clang-format off
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t delete_all_node = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(DELETE_ALL_REQUESTED));
  command_status_values_t state = FINAL_STATE;
  attribute_store_set_reported(delete_all_node,&state,sizeof(state));
  state = NEEDS_ONE_COMMAND;
  attribute_store_set_desired(delete_all_node,&state,sizeof(state));
  // clang-format on

  on_delete_all_send_data_complete(delete_all_node,
                                   RESOLVER_GET_RULE,
                                   FRAME_SENT_EVENT_OK_SUPERVISION_WORKING);

  TEST_ASSERT_EQUAL(FINAL_STATE,
                    attribute_store_get_reported_number(delete_all_node));
  TEST_ASSERT_EQUAL(FINAL_STATE,
                    attribute_store_get_desired_number(delete_all_node));
}

void test_zwave_command_class_user_code_on_delete_all_complete_fail()
{
  // clang-format off
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t delete_all_node = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(DELETE_ALL_REQUESTED));
  command_status_values_t state = FINAL_STATE;
  attribute_store_set_reported(delete_all_node,&state,sizeof(state));
  state = NEEDS_ONE_COMMAND;
  attribute_store_set_desired(delete_all_node,&state,sizeof(state));
  attribute_store_node_t user_id_node = create_user_code_nodes(13320);
  attribute_store_node_t user_code_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));
  attribute_store_node_t user_id_status_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(USER_ID_STATUS));
  const char code[] = "1234";
  attribute_store_set_reported(user_code_node, code, sizeof(code));
  uint8_t user_id_status = 1;
  attribute_store_set_reported(user_id_status_node, &user_id_status, sizeof(user_id_status));
  attribute_store_node_t user_id_node_2 = create_user_code_nodes(1);
  attribute_store_node_t user_code_node_2 = attribute_store_get_first_child_by_type(user_id_node_2, ATTRIBUTE(CODE));
  attribute_store_node_t user_id_status_node_2 = attribute_store_get_first_child_by_type(user_id_node_2, ATTRIBUTE(USER_ID_STATUS));
  attribute_store_set_reported(user_code_node_2, code, sizeof(code));
  attribute_store_set_reported(user_id_status_node_2, &user_id_status, sizeof(user_id_status));
  // clang-format on

  // WORKING: WIll do nothing, just wait
  on_delete_all_send_data_complete(delete_all_node,
                                   RESOLVER_SET_RULE,
                                   FRAME_SENT_EVENT_OK_SUPERVISION_WORKING);

  TEST_ASSERT_EQUAL(FINAL_STATE,
                    attribute_store_get_reported_number(delete_all_node));
  TEST_ASSERT_EQUAL(NEEDS_ONE_COMMAND,
                    attribute_store_get_desired_number(delete_all_node));
  TEST_ASSERT_EQUAL(1,
                    attribute_store_get_reported_number(user_id_status_node));
  TEST_ASSERT_EQUAL(1,
                    attribute_store_get_reported_number(user_id_status_node_2));

  // FAIL: Will roll back the Delete All Requested attribute, undefine all existing codes
  on_delete_all_send_data_complete(delete_all_node,
                                   RESOLVER_SET_RULE,
                                   FRAME_SENT_EVENT_OK_SUPERVISION_FAIL);

  TEST_ASSERT_EQUAL(FINAL_STATE,
                    attribute_store_get_reported_number(delete_all_node));
  TEST_ASSERT_EQUAL(FINAL_STATE,
                    attribute_store_get_desired_number(delete_all_node));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(user_id_status_node));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(user_id_status_node_2));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(user_id_status_node));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(user_id_status_node_2));
}

void test_zwave_command_class_user_code_on_delete_all_complete()
{
  // clang-format off
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t delete_all_node = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(DELETE_ALL_REQUESTED));
  command_status_values_t state = FINAL_STATE;
  attribute_store_set_reported(delete_all_node,&state,sizeof(state));
  state = NEEDS_ONE_COMMAND;
  attribute_store_set_desired(delete_all_node,&state,sizeof(state));
  attribute_store_node_t user_id_node = create_user_code_nodes(13320);
  attribute_store_node_t user_code_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));
  attribute_store_node_t user_id_status_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(USER_ID_STATUS));
  const char code[] = "1234";
  attribute_store_set_reported(user_code_node, code, sizeof(code));
  uint8_t user_id_status = 1;
  attribute_store_set_reported(user_id_status_node, &user_id_status, sizeof(user_id_status));
  attribute_store_node_t user_id_node_2 = create_user_code_nodes(1);
  attribute_store_node_t user_code_node_2 = attribute_store_get_first_child_by_type(user_id_node_2, ATTRIBUTE(CODE));
  attribute_store_node_t user_id_status_node_2 = attribute_store_get_first_child_by_type(user_id_node_2, ATTRIBUTE(USER_ID_STATUS));
  attribute_store_set_reported(user_code_node_2, code, sizeof(code));
  attribute_store_set_reported(user_id_status_node_2, &user_id_status, sizeof(user_id_status));
  // clang-format on

  // No error: Will undefine all user codes
  on_delete_all_send_data_complete(delete_all_node,
                                   RESOLVER_SET_RULE,
                                   FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS);

  TEST_ASSERT_EQUAL(FINAL_STATE,
                    attribute_store_get_reported_number(delete_all_node));
  TEST_ASSERT_EQUAL(FINAL_STATE,
                    attribute_store_get_desired_number(delete_all_node));
  TEST_ASSERT_EQUAL(0,
                    attribute_store_get_reported_number(user_id_status_node));
  TEST_ASSERT_EQUAL(0,
                    attribute_store_get_reported_number(user_id_status_node_2));

  char new_code[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
  attribute_store_get_reported_string(user_code_node,
                                      new_code,
                                      sizeof(new_code));
  TEST_ASSERT_EQUAL_STRING("", new_code);
  attribute_store_get_reported_string(user_code_node_2,
                                      new_code,
                                      sizeof(new_code));
  TEST_ASSERT_EQUAL_STRING("", new_code);
}

void test_zwave_command_class_user_code_incoming_invalid_command_class()
{
  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_SWITCH_BINARY, USER_CODE_REPORT_V2};
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));
}

void test_zwave_command_class_user_code_incoming_invalid_command()
{
  const uint8_t incoming_report_frame[] = {COMMAND_CLASS_USER_CODE_V2, 0xFF};
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));
}

void test_zwave_command_class_user_code_incoming_frame_too_short()
{
  const uint8_t incoming_report_frame[] = {COMMAND_CLASS_USER_CODE_V2};
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));
}

void test_zwave_command_class_user_code_incoming_user_code_report()
{
  // clang-format off
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t user_id_node = create_user_code_nodes(4);
  attribute_store_node_t user_code_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));
  attribute_store_node_t user_id_status_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(USER_ID_STATUS));
  // clang-format on
  const uint8_t incoming_report_frame[] = {COMMAND_CLASS_USER_CODE_V2,
                                           USER_CODE_REPORT_V2,
                                           4,
                                           2,
                                           0x30,
                                           0x32,
                                           0x33,
                                           0x34,
                                           0x35};
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  TEST_ASSERT_EQUAL(2,
                    attribute_store_get_reported_number(user_id_status_node));
  char saved_code[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
  attribute_store_get_reported_string(user_code_node,
                                      saved_code,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  TEST_ASSERT_EQUAL_STRING("02345", saved_code);
}

void test_zwave_command_class_user_code_incoming_user_code_report_available_user_code_without_code()
{
  // clang-format off
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t user_id_node = create_user_code_nodes(4);
  attribute_store_node_t user_code_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));
  attribute_store_node_t user_id_status_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(USER_ID_STATUS));
  // clang-format on
  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, USER_CODE_REPORT_V2, 4, 0};
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  TEST_ASSERT_EQUAL(0,
                    attribute_store_get_reported_number(user_id_status_node));
  char saved_code[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
  attribute_store_get_reported_string(user_code_node,
                                      saved_code,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  TEST_ASSERT_EQUAL_STRING("", saved_code);
}

void test_zwave_command_class_user_code_incoming_user_code_report_too_short()
{
  // clang-format off
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t user_id_node = create_user_code_nodes(2);
  attribute_store_node_t user_code_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(CODE));
  attribute_store_node_t user_id_status_node = attribute_store_get_first_child_by_type(user_id_node, ATTRIBUTE(USER_ID_STATUS));
  // clang-format on
  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, USER_CODE_REPORT_V2, 2};
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  TEST_ASSERT_EQUAL(0,
                    attribute_store_get_reported_number(user_id_status_node));
  char saved_code[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
  attribute_store_get_reported_string(user_code_node,
                                      saved_code,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  TEST_ASSERT_EQUAL_STRING("", saved_code);
}

void test_zwave_command_class_user_code_incoming_admin_code_report_too_short()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  // clang-format on

  // Example capabilities report from CTT (Expected Admin Code will be created)
  receive_ctt_v2_capabilities();
  attribute_store_node_t admin_code_node
    = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(ADMIN_CODE));
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, admin_code_node);

  // Now admin code too short comes in.
  const uint8_t incoming_report_frame[] = {COMMAND_CLASS_USER_CODE_V2, 0x10};
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  char saved_code[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
  attribute_store_get_reported_string(admin_code_node,
                                      saved_code,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  TEST_ASSERT_EQUAL_STRING("", saved_code);
}

void test_zwave_command_class_user_code_incoming_admin_code_report_malicious_length_1()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  // clang-format on

  // Example capabilities report from CTT (Expected Admin Code will be created)
  receive_ctt_v2_capabilities();
  attribute_store_node_t admin_code_node
    = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(ADMIN_CODE));
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, admin_code_node);

  // Now admin code comes in, indicates length 255 but is only a few bytes.
  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, 0x10, 255, 0x30, 0x32, 0x30, 0x32};
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  char saved_code[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
  attribute_store_get_reported_string(admin_code_node,
                                      saved_code,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  TEST_ASSERT_EQUAL_STRING("", saved_code);
}

void test_zwave_command_class_user_code_incoming_admin_code_report_malicious_length_2()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  // clang-format on

  // Example capabilities report from CTT (Expected Admin Code will be created)
  receive_ctt_v2_capabilities();
  attribute_store_node_t admin_code_node
    = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(ADMIN_CODE));
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, admin_code_node);

  // Now admin code comes in, indicates length 2 but is only a 1 byte.
  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, 0x10, 2, 0x30};
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  char saved_code[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
  attribute_store_get_reported_string(admin_code_node,
                                      saved_code,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  TEST_ASSERT_EQUAL_STRING("", saved_code);
}

void test_zwave_command_class_user_code_incoming_admin_code_report_happy_case()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  // clang-format on

  // Example capabilities report from CTT (Expected Admin Code will be created)
  receive_ctt_v2_capabilities();
  attribute_store_node_t admin_code_node
    = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(ADMIN_CODE));
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, admin_code_node);

  // Now admin code comes in, indicates length 2 but is only a 1 byte.
  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, 0x10, 6, 0x30, 0x39, 0x33, 0x30, 0x38, 0x41};
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  char saved_code[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
  attribute_store_get_reported_string(admin_code_node,
                                      saved_code,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  TEST_ASSERT_EQUAL_STRING("09308A", saved_code);
}

void test_zwave_command_class_user_code_incoming_extended_user_code_happy_case()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  #define NB_OF_CODES 20
  attribute_store_node_t user_id_nodes[NB_OF_CODES] = {};
  attribute_store_node_t user_code_nodes[NB_OF_CODES] = {};
  attribute_store_node_t user_id_status_nodes[NB_OF_CODES] = {};
  const uint8_t default_status = 3;
  const char default_code[] = "1234";
  for (int i = 0; i< NB_OF_CODES; i++) {
    user_id_nodes[i] = create_user_code_nodes(i+1);
    user_code_nodes[i] = attribute_store_get_first_child_by_type(user_id_nodes[i], ATTRIBUTE(CODE));
    attribute_store_set_reported(user_code_nodes[i], default_code, sizeof(default_code));
    user_id_status_nodes[i] = attribute_store_get_first_child_by_type(user_id_nodes[i], ATTRIBUTE(USER_ID_STATUS));
    attribute_store_set_reported(user_id_status_nodes[i], &default_status, sizeof(default_status));
  }
  receive_ctt_v2_capabilities();
  // clang-format on

  // Now admin code comes in, indicates length 2 but is only a 1 byte.
  const uint8_t incoming_report_frame[] = {COMMAND_CLASS_USER_CODE_V2,
                                           EXTENDED_USER_CODE_REPORT_V2,
                                           2,  // 2 codes
                                           0,  // User ID 3 MSB
                                           3,  // User ID 3 LSB
                                           2,  // User ID 3 status
                                           4,  // Length
                                           0x44,
                                           0x43,
                                           0x42,
                                           0x41,
                                           0,   // User ID 6 MSB
                                           6,   // User ID 6 LSB
                                           20,  // User ID 6 status
                                           4,   // Length
                                           0x34,
                                           0x33,
                                           0x32,
                                           0x31,
                                           0,  // User ID 12 is next
                                           12};
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  char saved_code[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
  // User code 2 should be unchanged:
  TEST_ASSERT_EQUAL(
    default_status,
    attribute_store_get_reported_number(user_id_status_nodes[1]));
  attribute_store_get_reported_string(user_code_nodes[1],
                                      saved_code,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  TEST_ASSERT_EQUAL_STRING(default_code, saved_code);

  // User code 3 has a new code / status
  TEST_ASSERT_EQUAL(
    2,
    attribute_store_get_reported_number(user_id_status_nodes[2]));
  attribute_store_get_reported_string(user_code_nodes[2],
                                      saved_code,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  TEST_ASSERT_EQUAL_STRING("DCBA", saved_code);

  // User code 4, 5 should have been erased and set to available.
  TEST_ASSERT_EQUAL(
    0,
    attribute_store_get_reported_number(user_id_status_nodes[3]));
  TEST_ASSERT_EQUAL(
    0,
    attribute_store_get_reported_number(user_id_status_nodes[4]));
  attribute_store_get_reported_string(user_code_nodes[3],
                                      saved_code,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  TEST_ASSERT_EQUAL_STRING("", saved_code);
  attribute_store_get_reported_string(user_code_nodes[4],
                                      saved_code,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  TEST_ASSERT_EQUAL_STRING("", saved_code);

  // User code 6 has a new code/status
  TEST_ASSERT_EQUAL(
    20,
    attribute_store_get_reported_number(user_id_status_nodes[5]));
  attribute_store_get_reported_string(user_code_nodes[5],
                                      saved_code,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  TEST_ASSERT_EQUAL_STRING("4321", saved_code);
  attribute_store_get_reported_string(user_code_nodes[4],
                                      saved_code,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);

  // Anything until 12 is also set to "unavailable"
  for (int i = 6; i < 11; i++) {
    TEST_ASSERT_EQUAL(
      0,
      attribute_store_get_reported_number(user_id_status_nodes[i]));

    attribute_store_get_reported_string(user_code_nodes[i],
                                        saved_code,
                                        ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
    TEST_ASSERT_EQUAL_STRING("", saved_code);
  }

  // User ID 12 is untouched:
  TEST_ASSERT_EQUAL(
    default_status,
    attribute_store_get_reported_number(user_id_status_nodes[11]));
  attribute_store_get_reported_string(user_code_nodes[11],
                                      saved_code,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  TEST_ASSERT_EQUAL_STRING(default_code, saved_code);
}

void test_zwave_command_class_user_code_incoming_extended_user_code_too_short()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  #define NB_OF_CODES 20
  attribute_store_node_t user_id_nodes[NB_OF_CODES] = {};
  attribute_store_node_t user_code_nodes[NB_OF_CODES] = {};
  attribute_store_node_t user_id_status_nodes[NB_OF_CODES] = {};
  const uint8_t default_status = 3;
  const char default_code[] = "1234";
  for (int i = 0; i< NB_OF_CODES; i++) {
    user_id_nodes[i] = create_user_code_nodes(i+1);
    user_code_nodes[i] = attribute_store_get_first_child_by_type(user_id_nodes[i], ATTRIBUTE(CODE));
    attribute_store_set_reported(user_code_nodes[i], default_code, sizeof(default_code));
    user_id_status_nodes[i] = attribute_store_get_first_child_by_type(user_id_nodes[i], ATTRIBUTE(USER_ID_STATUS));
    attribute_store_set_reported(user_id_status_nodes[i], &default_status, sizeof(default_status));
  }
  receive_ctt_v2_capabilities();
  // clang-format on

  const uint8_t incoming_report_frame[] = {
    COMMAND_CLASS_USER_CODE_V2,
    EXTENDED_USER_CODE_REPORT_V2,
    2,  // 2 codes
    0,  // User ID 3 MSB
    3,  // User ID 3 LSB
    2,  // User ID 3 status
    4,  // Length
    0x44,
    0x43,
    0x42,
    0x41,
    0,   // User ID 6 MSB
    6,   // User ID 6 LSB
    20,  // User ID 6 status
    6,   // Length
    0x34,
    0x33,
    0x32,  // This will be interpreted as Next User ID
    0x31   // This will be interpreted as Next User ID
  };
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  char saved_code[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
  // User code 2 should be unchanged:
  TEST_ASSERT_EQUAL(
    default_status,
    attribute_store_get_reported_number(user_id_status_nodes[1]));
  attribute_store_get_reported_string(user_code_nodes[1],
                                      saved_code,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  TEST_ASSERT_EQUAL_STRING(default_code, saved_code);

  // User code 3 has a new code / status
  TEST_ASSERT_EQUAL(
    2,
    attribute_store_get_reported_number(user_id_status_nodes[2]));
  attribute_store_get_reported_string(user_code_nodes[2],
                                      saved_code,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  TEST_ASSERT_EQUAL_STRING("DCBA", saved_code);

  // User code 4, 5 should have been erased and set to available.
  TEST_ASSERT_EQUAL(
    0,
    attribute_store_get_reported_number(user_id_status_nodes[3]));
  TEST_ASSERT_EQUAL(
    0,
    attribute_store_get_reported_number(user_id_status_nodes[4]));
  attribute_store_get_reported_string(user_code_nodes[3],
                                      saved_code,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  TEST_ASSERT_EQUAL_STRING("", saved_code);
  attribute_store_get_reported_string(user_code_nodes[4],
                                      saved_code,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  TEST_ASSERT_EQUAL_STRING("", saved_code);

  // User code 6 should be unchanged, it was not valid.
  // Next User id indicates to wipe the rest of the users.
  for (int i = 5; i < NB_OF_CODES; i++) {
    attribute_store_get_reported_string(user_code_nodes[i],
                                        saved_code,
                                        ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
    TEST_ASSERT_EQUAL_STRING("", saved_code);
  }
}

void test_zwave_command_class_user_code_incoming_extended_wrong_number_of_users()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  #define NB_OF_CODES 20
  attribute_store_node_t user_id_nodes[NB_OF_CODES] = {};
  attribute_store_node_t user_code_nodes[NB_OF_CODES] = {};
  attribute_store_node_t user_id_status_nodes[NB_OF_CODES] = {};
  const uint8_t default_status = 3;
  const char default_code[] = "1234";
  for (int i = 0; i< NB_OF_CODES; i++) {
    user_id_nodes[i] = create_user_code_nodes(i+1);
    user_code_nodes[i] = attribute_store_get_first_child_by_type(user_id_nodes[i], ATTRIBUTE(CODE));
    attribute_store_set_reported(user_code_nodes[i], default_code, sizeof(default_code));
    user_id_status_nodes[i] = attribute_store_get_first_child_by_type(user_id_nodes[i], ATTRIBUTE(USER_ID_STATUS));
    attribute_store_set_reported(user_id_status_nodes[i], &default_status, sizeof(default_status));
  }
  receive_ctt_v2_capabilities();
  // clang-format on

  const uint8_t incoming_report_frame[] = {
    COMMAND_CLASS_USER_CODE_V2,
    EXTENDED_USER_CODE_REPORT_V2,
    4,  // 4 codes, but just 1 in the payload
    0,  // User ID 3 MSB
    3,  // User ID 3 LSB
    2,  // User ID 3 status
    4,  // Length
    0x44,
    0x43,
    0x42,
    0x41,
    0,  // This will be interpreted as Next User ID
    0,  // This will be interpreted as Next User ID
  };
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  char saved_code[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
  // User code 3 has a new code / status
  TEST_ASSERT_EQUAL(
    2,
    attribute_store_get_reported_number(user_id_status_nodes[2]));
  attribute_store_get_reported_string(user_code_nodes[2],
                                      saved_code,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  TEST_ASSERT_EQUAL_STRING("DCBA", saved_code);

  // User code 4 onwards should be wiped.
  for (int i = 3; i < NB_OF_CODES; i++) {
    attribute_store_get_reported_string(user_code_nodes[i],
                                        saved_code,
                                        ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
    TEST_ASSERT_EQUAL_STRING(default_code, saved_code);
  }
}

void test_zwave_command_class_user_code_incoming_extended_too_short()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  #define NB_OF_CODES 20
  attribute_store_node_t user_id_nodes[NB_OF_CODES] = {};
  attribute_store_node_t user_code_nodes[NB_OF_CODES] = {};
  attribute_store_node_t user_id_status_nodes[NB_OF_CODES] = {};
  const uint8_t default_status = 3;
  const char default_code[] = "1234";
  for (int i = 0; i< NB_OF_CODES; i++) {
    user_id_nodes[i] = create_user_code_nodes(i+1);
    user_code_nodes[i] = attribute_store_get_first_child_by_type(user_id_nodes[i], ATTRIBUTE(CODE));
    attribute_store_set_reported(user_code_nodes[i], default_code, sizeof(default_code));
    user_id_status_nodes[i] = attribute_store_get_first_child_by_type(user_id_nodes[i], ATTRIBUTE(USER_ID_STATUS));
    attribute_store_set_reported(user_id_status_nodes[i], &default_status, sizeof(default_status));
  }
  receive_ctt_v2_capabilities();
  // clang-format on

  const uint8_t incoming_report_frame[] = {
    COMMAND_CLASS_USER_CODE_V2,
    EXTENDED_USER_CODE_REPORT_V2,
    4,  // 4 codes, but just 1 in the payload
    0,  // User ID 3 MSB
    3,  // User ID 3 LSB
    2   // User ID 3 status
  };
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  char saved_code[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];

  // User code 4 onwards should be wiped.
  for (int i = 0; i < NB_OF_CODES; i++) {
    attribute_store_get_reported_string(user_code_nodes[i],
                                        saved_code,
                                        ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
    TEST_ASSERT_EQUAL_STRING("1234", saved_code);
    TEST_ASSERT_EQUAL(
      default_status,
      attribute_store_get_reported_number(user_id_status_nodes[2]));
  }
}

void test_zwave_command_class_user_code_incoming_user_number_report()
{
  // clang-format off
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  // clang-format on

  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, USERS_NUMBER_REPORT_V2};
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  attribute_store_node_t number_of_users_node
    = attribute_store_get_first_child_by_type(data_node,
                                              ATTRIBUTE(NUMBER_OF_USERS));

  TEST_ASSERT_EQUAL(0,
                    attribute_store_get_reported_number(number_of_users_node));
}

void test_zwave_command_class_user_code_incoming_user_number_report_v1()
{
  // clang-format off
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  // clang-format on

  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, USERS_NUMBER_REPORT_V2, 20, 20};
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  attribute_store_node_t number_of_users_node
    = attribute_store_get_first_child_by_type(data_node,
                                              ATTRIBUTE(NUMBER_OF_USERS));

  TEST_ASSERT_EQUAL(20,
                    attribute_store_get_reported_number(number_of_users_node));
}

void test_zwave_command_class_user_code_incoming_user_number_report_v2()
{
  // clang-format off
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  // clang-format on

  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, USERS_NUMBER_REPORT_V2, 20, 0x10, 0x12};
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  attribute_store_node_t number_of_users_node
    = attribute_store_get_first_child_by_type(data_node,
                                              ATTRIBUTE(NUMBER_OF_USERS));

  TEST_ASSERT_EQUAL(0x1012,
                    attribute_store_get_reported_number(number_of_users_node));
}

void test_zwave_command_class_user_code_incoming_checksum_report_v2()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  // clang-format on

  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, USER_CODE_CHECKSUM_REPORT_V2, 0x10, 0x12};
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  attribute_store_node_t checksum_node
    = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(CHECKSUM));

  TEST_ASSERT_EQUAL(0x1012, attribute_store_get_reported_number(checksum_node));
}

void test_zwave_command_class_user_code_incoming_checksum_report_too_short()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION),&version,sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  // clang-format on

  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, USER_CODE_CHECKSUM_REPORT_V2};
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  attribute_store_node_t checksum_node
    = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(CHECKSUM));

  TEST_ASSERT_EQUAL(0, attribute_store_get_reported_number(checksum_node));
}

void test_zwave_command_class_user_code_incoming_keypad_mode_report_too_short()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION), &version, sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  // clang-format on

  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, USER_CODE_KEYPAD_MODE_REPORT_V2};
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  attribute_store_node_t keypad_mode_node
    = attribute_store_get_first_child_by_type(data_node,
                                              ATTRIBUTE(KEYPAD_MODE));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(keypad_mode_node));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(keypad_mode_node));
}

void test_zwave_command_class_user_code_incoming_keypad_mode_report_happy_case()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION), &version, sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  // clang-format on

  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_USER_CODE_V2, USER_CODE_KEYPAD_MODE_REPORT_V2, 03};
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  attribute_store_node_t keypad_mode_node
    = attribute_store_get_first_child_by_type(data_node,
                                              ATTRIBUTE(KEYPAD_MODE));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(keypad_mode_node));
  TEST_ASSERT_EQUAL(3, attribute_store_get_reported_number(keypad_mode_node));
}

void test_zwave_command_class_user_code_incoming_capabilities_report_only_flags()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION), &version, sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t capabilities_node = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(CAPABILITIES));
  // clang-format on

  const uint8_t incoming_report_frame[] = {COMMAND_CLASS_USER_CODE_V2,
                                           USER_CODE_CAPABILITIES_REPORT_V2,
                                           0x80,
                                           0x80,
                                           0};
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  attribute_store_node_t flags_node
    = attribute_store_get_first_child_by_type(capabilities_node,
                                              ATTRIBUTE(SUPPORTED_FLAGS));

  uint8_t flags = 0;
  attribute_store_get_reported(flags_node, &flags, sizeof(flags));
  TEST_ASSERT_EQUAL(1, flags & 1);
  TEST_ASSERT_EQUAL((1 << 3), flags & (1 << 3));
}

void test_zwave_command_class_user_code_incoming_capabilities_dangerous_bitmask_length()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION), &version, sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t capabilities_node = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(CAPABILITIES));
  // clang-format on

  const uint8_t incoming_report_frame[] = {COMMAND_CLASS_USER_CODE_V2,
                                           USER_CODE_CAPABILITIES_REPORT_V2,
                                           0xFF,
                                           0xFF,
                                           0xFF};
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  attribute_store_node_t flags_node
    = attribute_store_get_first_child_by_type(capabilities_node,
                                              ATTRIBUTE(SUPPORTED_FLAGS));
  uint8_t flags = 0;
  attribute_store_get_reported(flags_node, &flags, sizeof(flags));
  TEST_ASSERT_EQUAL(flags & 1, 1);
  TEST_ASSERT_EQUAL(flags & (1 << 1), (1 << 1));
  TEST_ASSERT_EQUAL(flags & (1 << 2), (1 << 2));

  attribute_store_node_t user_id_status_bitmask_node
    = attribute_store_get_first_child_by_type(
      capabilities_node,
      ATTRIBUTE(SUPPORTED_USER_ID_STATUS));
  uint8_t value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  uint8_t value_size                                  = 0;
  attribute_store_get_node_attribute_value(user_id_status_bitmask_node,
                                           REPORTED_ATTRIBUTE,
                                           value,
                                           &value_size);
  TEST_ASSERT_EQUAL(1, value_size);
  TEST_ASSERT_EQUAL(7, value[0]);

  attribute_store_node_t keypad_mode_node
    = attribute_store_get_first_child_by_type(
      capabilities_node,
      ATTRIBUTE(SUPPORTED_KEYPAD_MODES));
  attribute_store_get_node_attribute_value(keypad_mode_node,
                                           REPORTED_ATTRIBUTE,
                                           value,
                                           &value_size);
  TEST_ASSERT_EQUAL(1, value_size);
  TEST_ASSERT_EQUAL(1, value[0]);

  attribute_store_node_t keys_node
    = attribute_store_get_first_child_by_type(capabilities_node,
                                              ATTRIBUTE(SUPPORTED_KEYS));
  attribute_store_get_node_attribute_value(keys_node,
                                           REPORTED_ATTRIBUTE,
                                           value,
                                           &value_size);

  TEST_ASSERT_EQUAL(KEY_BITMASK_SIZE, value_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(V1_SUPPORTED_KEYS, value, value_size);

  // Admin code is supported, it should have been created
  attribute_store_node_t admin_code_node
    = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(ADMIN_CODE));
  TEST_ASSERT_TRUE(attribute_store_node_exists(admin_code_node));

  // User Code checksum should not have been created
  attribute_store_node_t checksum_node
    = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(CHECKSUM));
  TEST_ASSERT_FALSE(attribute_store_node_exists(checksum_node));
}

void test_zwave_command_class_user_code_incoming_capabilities_happy_case()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION), &version, sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t capabilities_node = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(CAPABILITIES));
  // clang-format on

  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_USER_CODE_V2,
       USER_CODE_CAPABILITIES_REPORT_V2,
       0x41,  // ACD Support, bitmask length 1
       0x17,  // Supported User ID statuses
       0xE2,  // UCC / MUCR / MUCS support, length 2
       0x07,
       0x07,
       0x0A,  // Length 10
       0x01,
       0x02,
       0x03,
       0x04,
       0x05,
       0x06,
       0xFF,
       0x03,
       0xFE,
       0xFF};
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  attribute_store_node_t flags_node
    = attribute_store_get_first_child_by_type(capabilities_node,
                                              ATTRIBUTE(SUPPORTED_FLAGS));
  uint8_t flags = 0;
  attribute_store_get_reported(flags_node, &flags, sizeof(flags));
  TEST_ASSERT_EQUAL(flags & 1, 0);
  TEST_ASSERT_EQUAL(flags & (1 << 1), (1 << 1));
  TEST_ASSERT_EQUAL(flags & (1 << 2), 0);
  TEST_ASSERT_EQUAL(flags & (1 << 3), (1 << 3));
  TEST_ASSERT_EQUAL(flags & (1 << 4), (1 << 4));
  TEST_ASSERT_EQUAL(flags & (1 << 5), (1 << 5));

  attribute_store_node_t user_id_status_bitmask_node
    = attribute_store_get_first_child_by_type(
      capabilities_node,
      ATTRIBUTE(SUPPORTED_USER_ID_STATUS));
  uint8_t value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  uint8_t value_size                                  = 0;
  attribute_store_get_node_attribute_value(user_id_status_bitmask_node,
                                           REPORTED_ATTRIBUTE,
                                           value,
                                           &value_size);
  TEST_ASSERT_EQUAL(1, value_size);
  TEST_ASSERT_EQUAL(0x17, value[0]);

  attribute_store_node_t keypad_mode_node
    = attribute_store_get_first_child_by_type(
      capabilities_node,
      ATTRIBUTE(SUPPORTED_KEYPAD_MODES));
  attribute_store_get_node_attribute_value(keypad_mode_node,
                                           REPORTED_ATTRIBUTE,
                                           value,
                                           &value_size);
  TEST_ASSERT_EQUAL(2, value_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(&incoming_report_frame[5], value, value_size);

  attribute_store_node_t keys_node
    = attribute_store_get_first_child_by_type(capabilities_node,
                                              ATTRIBUTE(SUPPORTED_KEYS));
  attribute_store_get_node_attribute_value(keys_node,
                                           REPORTED_ATTRIBUTE,
                                           value,
                                           &value_size);

  TEST_ASSERT_EQUAL(10, value_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(&incoming_report_frame[8], value, value_size);

  // Admin code is not supported, it should not have been created
  attribute_store_node_t admin_code_node
    = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(ADMIN_CODE));
  TEST_ASSERT_FALSE(attribute_store_node_exists(admin_code_node));

  // User Code checksum should have been created
  attribute_store_node_t checksum_node
    = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(CHECKSUM));
  TEST_ASSERT_TRUE(attribute_store_node_exists(checksum_node));
}

void test_zwave_command_class_user_code_incoming_capabilities_too_short()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION), &version, sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t capabilities_node = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(CAPABILITIES));
  // clang-format on

  const uint8_t incoming_report_frame[] = {
    COMMAND_CLASS_USER_CODE_V2,
    USER_CODE_CAPABILITIES_REPORT_V2,
    0x01,  // ACD Support, bitmask length 1
    0x17   // Supported User ID statuses
  };
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  attribute_store_node_t flags_node
    = attribute_store_get_first_child_by_type(capabilities_node,
                                              ATTRIBUTE(SUPPORTED_FLAGS));
  uint8_t flags = 0xFF;
  attribute_store_get_reported(flags_node, &flags, sizeof(flags));
  TEST_ASSERT_EQUAL(flags & 1, 0);
  TEST_ASSERT_EQUAL(flags & (1 << 1), 0);
  TEST_ASSERT_EQUAL(flags & (1 << 2), 0);
  TEST_ASSERT_EQUAL(flags & (1 << 3), 0);
  TEST_ASSERT_EQUAL(flags & (1 << 4), 0);
  TEST_ASSERT_EQUAL(flags & (1 << 5), 0);

  attribute_store_node_t user_id_status_bitmask_node
    = attribute_store_get_first_child_by_type(
      capabilities_node,
      ATTRIBUTE(SUPPORTED_USER_ID_STATUS));
  uint8_t value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  uint8_t value_size                                  = 0;
  attribute_store_get_node_attribute_value(user_id_status_bitmask_node,
                                           REPORTED_ATTRIBUTE,
                                           value,
                                           &value_size);
  TEST_ASSERT_EQUAL(1, value_size);
  TEST_ASSERT_EQUAL(0x07, value[0]);

  attribute_store_node_t keypad_mode_node
    = attribute_store_get_first_child_by_type(
      capabilities_node,
      ATTRIBUTE(SUPPORTED_KEYPAD_MODES));
  attribute_store_get_node_attribute_value(keypad_mode_node,
                                           REPORTED_ATTRIBUTE,
                                           value,
                                           &value_size);

  TEST_ASSERT_EQUAL(1, value_size);
  TEST_ASSERT_EQUAL(0x1, value[0]);

  attribute_store_node_t keys_node
    = attribute_store_get_first_child_by_type(capabilities_node,
                                              ATTRIBUTE(SUPPORTED_KEYS));
  attribute_store_get_node_attribute_value(keys_node,
                                           REPORTED_ATTRIBUTE,
                                           value,
                                           &value_size);

  TEST_ASSERT_EQUAL(KEY_BITMASK_SIZE, value_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(V1_SUPPORTED_KEYS, value, value_size);
}

void test_zwave_command_class_user_code_incoming_capabilities_wrong_length_indicator()
{
  // clang-format off
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION), &version, sizeof(version));
  attribute_store_node_t data_node = attribute_store_get_first_child_by_type(endpoint_id_node, ATTRIBUTE(DATA));
  attribute_store_node_t capabilities_node = attribute_store_get_first_child_by_type(data_node, ATTRIBUTE(CAPABILITIES));
  // clang-format on

  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_USER_CODE_V2,
       USER_CODE_CAPABILITIES_REPORT_V2,
       0x01,  // ACD Support, bitmask length 1
       0x17,  // Supported User ID statuses
       0x02,  // UCC / MUCR / MUCS support, length 2
       0x07,
       0x07,
       0x0A,  // Length 10
       0x01,
       0x02,
       0x03,
       0x04,
       0x05,
       0x06,
       0xFF,
       0x03,
       0xFE};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  attribute_store_node_t flags_node
    = attribute_store_get_first_child_by_type(capabilities_node,
                                              ATTRIBUTE(SUPPORTED_FLAGS));
  uint8_t flags = 0xFF;
  attribute_store_get_reported(flags_node, &flags, sizeof(flags));
  TEST_ASSERT_EQUAL(flags & 1, 0);
  TEST_ASSERT_EQUAL(flags & (1 << 1), 0);
  TEST_ASSERT_EQUAL(flags & (1 << 2), 0);
  TEST_ASSERT_EQUAL(flags & (1 << 3), 0);
  TEST_ASSERT_EQUAL(flags & (1 << 4), 0);
  TEST_ASSERT_EQUAL(flags & (1 << 5), 0);

  attribute_store_node_t keys_node
    = attribute_store_get_first_child_by_type(capabilities_node,
                                              ATTRIBUTE(SUPPORTED_KEYS));
  uint8_t value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  uint8_t value_size                                  = 0;
  attribute_store_get_node_attribute_value(keys_node,
                                           REPORTED_ATTRIBUTE,
                                           value,
                                           &value_size);

  TEST_ASSERT_EQUAL(KEY_BITMASK_SIZE, value_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(V1_SUPPORTED_KEYS, value, value_size);
}

//*/