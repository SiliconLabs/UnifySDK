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
#include "zwave_command_class_version.h"

// Generic includes
#include <string.h>
#include <unity.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "zwave_controller_utils.h"
#include "zpc_attribute_store_type_registration.h"

// Interface includes
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zwave_utils.h"
#include "zwave_controller_types.h"
#include "zwave_command_class_indices.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "attribute_resolver_mock.h"
#include "zpc_attribute_resolver_mock.h"
#include "zwave_command_handler_mock.h"
#include "zwave_tx_mock.h"
#include "zwave_tx_scheme_selector_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_CC_VERSION_##type

// Static test variables
static attribute_resolver_function_t version_get               = NULL;
static attribute_resolver_function_t version_capabilities_get  = NULL;
static attribute_resolver_function_t version_software_get      = NULL;
static attribute_resolver_function_t version_cc_get            = NULL;
static void (*on_nif_attribute_update)(attribute_store_node_t) = NULL;
static zwave_command_handler_t version_handler                 = {};

static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;

static uint8_t u8_value   = 0;
static uint32_t u32_value = 0;

static zwave_controller_connection_info_t connection_info = {};
static uint8_t test_nif_content[]
  = {COMMAND_CLASS_NOTIFICATION_V4,
     COMMAND_CLASS_SWITCH_BINARY,
     COMMAND_CLASS_SECURITY_2,
     0x0C,  // Fake protocol CC. Should be ignored
     COMMAND_CLASS_DOOR_LOCK_LOGGING,
     COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V2,
     0xFA,  // 2 bytes, so the next byte is not VERSION CC
     COMMAND_CLASS_VERSION,
     COMMAND_CLASS_MULTI_CHANNEL_V2,
     COMMAND_CLASS_DOOR_LOCK};

static uint8_t test_secure_nif_content[]
  = {COMMAND_CLASS_NOTIFICATION_V4,
     COMMAND_CLASS_VERSION,
     COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V2,
     COMMAND_CLASS_MULTI_CHANNEL_V2,
     COMMAND_CLASS_DOOR_LOCK};

// static size_t test_nif_size
//   = sizeof(test_supported_cc) / sizeof(zwave_command_class_t);

// Stub functions
static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  if (node_type == ATTRIBUTE(VERSION_REPORT_DATA)) {
    version_get = get_func;
    TEST_ASSERT_NOT_NULL(get_func);
    TEST_ASSERT_NULL(set_func);
  } else if (node_type == ATTRIBUTE(APPLICATION_FRAMEWORK_API_VERSION)) {
    version_software_get = get_func;
    TEST_ASSERT_NOT_NULL(get_func);
    TEST_ASSERT_NULL(set_func);
  } else if (node_type == ATTRIBUTE(ZWAVE_SOFTWARE_GET_SUPPORT)) {
    version_capabilities_get = get_func;
    TEST_ASSERT_NOT_NULL(get_func);
    TEST_ASSERT_NULL(set_func);
  } else if (node_type == ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_BASIC)) {
    version_cc_get = get_func;
    TEST_ASSERT_NOT_NULL(get_func);
    TEST_ASSERT_NULL(set_func);
  } else {
    TEST_FAIL_MESSAGE("Attribute rule registration on a wrong type");
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  version_handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME,
                    version_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_VERSION_V3, version_handler.command_class);
  TEST_ASSERT_EQUAL(VERSION_VERSION_V3, version_handler.version);
  TEST_ASSERT_NOT_NULL(version_handler.control_handler);
  TEST_ASSERT_NOT_NULL(version_handler.support_handler);
  TEST_ASSERT_FALSE(version_handler.manual_security_validation);

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

static void zwave_command_class_version_init_verification()
{
  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_version_init());
}

/// Called before each and every test
void setUp()
{
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_set_home_id(home_id);

  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size      = 0;
  u8_value                 = 0;
  u32_value                = 0;
  version_get              = NULL;
  version_capabilities_get = NULL;
  version_software_get     = NULL;
  version_cc_get           = NULL;
  on_nif_attribute_update  = NULL;
  memset(&version_handler, 0, sizeof(zwave_command_handler_t));

  // Connection info for incoming frames from the other node.
  memset(&connection_info, 0, sizeof(zwave_controller_connection_info_t));
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  zwave_command_class_version_init_verification();
}

void test_on_zwave_cc_version_version_attribute_update_endpoint_0()
{
  // Create Command Class Version attribute under the endpoint 0:
  attribute_store_node_t endpoint_0_node
    = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, node_id_node);
  const zwave_endpoint_id_t endpoint_0 = 0;
  attribute_store_set_reported(endpoint_0_node,
                               &endpoint_0,
                               sizeof(endpoint_0));
  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(endpoint_0_node));

  // Nothing should happen on creation, missing value.
  attribute_store_node_t test_version_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_VERSION_VERSION,
                               endpoint_0_node);

  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_0_node));

  // Nothing happens either if no value is defined.
  u8_value = 1;
  attribute_store_set_reported(test_version_node, &u8_value, sizeof(u8_value));
  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(endpoint_0_node));

  u8_value = 3;
  attribute_store_set_reported(test_version_node, &u8_value, sizeof(u8_value));
  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_0_node));

  // Delete all these attributes
  attribute_store_delete_node(
    attribute_store_get_node_child_by_type(endpoint_0_node,
                                           ATTRIBUTE(VERSION_REPORT_DATA),
                                           0));
  attribute_store_delete_node(attribute_store_get_node_child_by_type(
    endpoint_0_node,
    ATTRIBUTE(ZWAVE_SOFTWARE_GET_SUPPORT),
    0));

  // Delete the test_node, nothing should be created again.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_0_node));
  attribute_store_delete_node(test_version_node);
  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(endpoint_0_node));
}

void test_on_zwave_cc_version_version_attribute_update_endpoint_3()
{
  // Create Command Class Version attribute under the endpoint:
  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(endpoint_id_node));

  // Nothing will happen with endpoints > 0
  attribute_store_node_t test_version_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_VERSION_VERSION,
                               endpoint_id_node);

  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  u8_value = 1;
  attribute_store_set_reported(test_version_node, &u8_value, sizeof(u8_value));
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  u8_value = 3;
  attribute_store_set_reported(test_version_node, &u8_value, sizeof(u8_value));
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  // Delete the test_node, nothing should be created again.
  attribute_store_delete_node(test_version_node);
  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(endpoint_id_node));
}

void test_on_nif_attribute_update()
{
  attribute_resolver_set_attribute_depth_IgnoreAndReturn(SL_STATUS_OK);
  zwave_endpoint_id_t endpoint_id = 0;
  attribute_store_set_reported(endpoint_id_node,
                               &endpoint_id,
                               sizeof(endpoint_id));
  // Create Command Class Version attribute under the endpoint:
  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(endpoint_id_node));

  // Create a NIF under the endpoint
  attribute_store_node_t test_nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);

  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  // Set up our NIF contents
  zwave_command_class_t supported_cc_list[ZWAVE_MAX_NIF_SIZE] = {0};
  size_t supported_cc_list_length                             = 0;
  zwave_parse_nif(test_nif_content,
                  sizeof(test_nif_content),
                  supported_cc_list,
                  &supported_cc_list_length,
                  NULL,
                  NULL);
  // Stop the stub, expect rule registrations for each Command Class.
  attribute_resolver_register_rule_Stub(NULL);

  attribute_resolver_register_rule_ExpectAndReturn(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_VERSION),
    NULL,
    version_cc_get,
    SL_STATUS_OK);

  for (size_t i = 0; i < supported_cc_list_length; i++) {
    if (supported_cc_list[i] >= COMMAND_CLASS_FIRST_APPLICATION_CC) {
      attribute_resolver_register_rule_ExpectAndReturn(
        ZWAVE_CC_VERSION_ATTRIBUTE(supported_cc_list[i]),
        NULL,
        version_cc_get,
        SL_STATUS_OK);
    }
  }

  // Fill up the NIF, version is not found yet:
  attribute_store_set_reported(test_nif_node,
                               test_nif_content,
                               sizeof(test_nif_content));

  for (size_t i = 0; i < supported_cc_list_length; i++) {
    attribute_store_node_t created_node
      = attribute_store_get_node_child_by_type(
        endpoint_id_node,
        ZWAVE_CC_VERSION_ATTRIBUTE(supported_cc_list[i]),
        0);
    if (supported_cc_list[i] < COMMAND_CLASS_FIRST_APPLICATION_CC) {
      TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, created_node);
    } else {
      zwave_cc_version_t default_version = 99;
      TEST_ASSERT_EQUAL(SL_STATUS_OK,
                        attribute_store_get_reported(created_node,
                                                     &default_version,
                                                     sizeof(default_version)));
      TEST_ASSERT_EQUAL(1, default_version);
    }
  }

  TEST_ASSERT_FALSE(is_version_cc_found(endpoint_id_node));

  // Add a Secure NIF with a version CC inside
  attribute_store_node_t test_secure_nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_SECURE_NIF, endpoint_id_node);

  // Set up our Secure NIF contents
  zwave_command_class_t supported_secure_cc_list[ZWAVE_MAX_NIF_SIZE] = {0};
  size_t supported_secure_cc_list_length                             = 0;
  zwave_parse_nif(test_secure_nif_content,
                  sizeof(test_secure_nif_content),
                  supported_secure_cc_list,
                  &supported_secure_cc_list_length,
                  NULL,
                  NULL);

  attribute_resolver_register_rule_ExpectAndReturn(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_VERSION),
    NULL,
    version_cc_get,
    SL_STATUS_OK);

  for (size_t i = 0; i < supported_cc_list_length; i++) {
    if (supported_cc_list[i] >= COMMAND_CLASS_FIRST_APPLICATION_CC) {
      attribute_resolver_register_rule_ExpectAndReturn(
        ZWAVE_CC_VERSION_ATTRIBUTE(supported_cc_list[i]),
        NULL,
        version_cc_get,
        SL_STATUS_OK);
    }
  }
  for (size_t i = 0; i < supported_secure_cc_list_length; i++) {
    if (supported_secure_cc_list[i] >= COMMAND_CLASS_FIRST_APPLICATION_CC) {
      attribute_resolver_register_rule_ExpectAndReturn(
        ZWAVE_CC_VERSION_ATTRIBUTE(supported_secure_cc_list[i]),
        NULL,
        version_cc_get,
        SL_STATUS_OK);
    }
  }
  // Set the content of our Secure NIF
  attribute_store_set_reported(test_secure_nif_node,
                               test_secure_nif_content,
                               sizeof(test_secure_nif_content));

  // Now all version CC attribute should be undefined
  for (size_t i = 0; i < supported_cc_list_length; i++) {
    attribute_store_node_t created_node
      = attribute_store_get_node_child_by_type(
        endpoint_id_node,
        ZWAVE_CC_VERSION_ATTRIBUTE(supported_cc_list[i]),
        0);
    if (supported_cc_list[i] < COMMAND_CLASS_FIRST_APPLICATION_CC) {
      TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, created_node);
    } else {
      TEST_ASSERT_FALSE(
        attribute_store_is_value_defined(created_node, REPORTED_ATTRIBUTE));
    }
  }
  for (size_t i = 0; i < supported_secure_cc_list_length; i++) {
    attribute_store_node_t created_node
      = attribute_store_get_node_child_by_type(
        endpoint_id_node,
        ZWAVE_CC_VERSION_ATTRIBUTE(supported_secure_cc_list[i]),
        0);
    if (supported_secure_cc_list[i] < COMMAND_CLASS_FIRST_APPLICATION_CC) {
      TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, created_node);
    } else {
      TEST_ASSERT_FALSE(
        attribute_store_is_value_defined(created_node, REPORTED_ATTRIBUTE));
    }
  }

  // Verify that there is a VERSION_FOUND attribute created
  attribute_store_node_t version_found
    = attribute_store_get_node_child_by_type(node_id_node, ATTRIBUTE(FOUND), 0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, version_found);

  // test with the API :
  TEST_ASSERT_TRUE(is_version_cc_found(endpoint_id_node));
}

void test_on_nif_attribute_update_no_endpoint_data()
{
  attribute_resolver_set_attribute_depth_IgnoreAndReturn(SL_STATUS_OK);
  attribute_store_undefine_reported(endpoint_id_node);
  // Create a NIF under the fake endpoint
  attribute_store_node_t test_nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);

  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  // Set up our NIF contents
  zwave_command_class_t supported_cc_list[ZWAVE_MAX_NIF_SIZE] = {0};
  size_t supported_cc_list_length                             = 0;
  zwave_parse_nif(test_nif_content,
                  sizeof(test_nif_content),
                  supported_cc_list,
                  &supported_cc_list_length,
                  NULL,
                  NULL);

  // Fill up the NIF, nothing should happen under an anonymous endpoint
  attribute_store_set_reported(test_nif_node,
                               test_nif_content,
                               sizeof(test_nif_content));

  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));
}

void test_zwave_command_class_version_unknown_command()
{
  // Simulate an incoming report, first too short:
  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(endpoint_id_node));
  const uint8_t incoming_frame[] = {COMMAND_CLASS_VERSION_V3, 0xFF};
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));
}

void test_zwave_command_class_version_unknown_command_class()
{
  // Simulate an incoming report, first too short:
  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(endpoint_id_node));
  const uint8_t incoming_frame[] = {COMMAND_CLASS_SECURITY_2, 0x01};
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));
}

void test_zwave_command_class_version_get()
{
  attribute_store_node_t version_report_node
    = attribute_store_add_node(ATTRIBUTE(VERSION_REPORT_DATA),
                               endpoint_id_node);

  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(version_get);
  version_get(version_report_node, received_frame, &received_frame_size);
  const uint8_t expected_frame[] = {COMMAND_CLASS_VERSION_V3, VERSION_GET_V3};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_version_software_get()
{
  attribute_store_node_t test_node
    = attribute_store_add_node(ATTRIBUTE(APPLICATION_FRAMEWORK_API_VERSION),
                               endpoint_id_node);

  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(version_software_get);
  version_software_get(test_node, received_frame, &received_frame_size);
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_VERSION_V3, VERSION_ZWAVE_SOFTWARE_GET_V3};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_version_capabilities_get()
{
  attribute_store_node_t test_node
    = attribute_store_add_node(ATTRIBUTE(ZWAVE_SOFTWARE_GET_SUPPORT),
                               endpoint_id_node);

  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(version_capabilities_get);
  version_capabilities_get(test_node, received_frame, &received_frame_size);
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_VERSION_V3, VERSION_CAPABILITIES_GET_V3};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_version_cc_get()
{
  attribute_store_node_t cc_version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_DOOR_LOCK),
    endpoint_id_node);

  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(version_cc_get);
  version_cc_get(cc_version_node, received_frame, &received_frame_size);
  const uint8_t expected_frame[] = {COMMAND_CLASS_VERSION_V3,
                                    VERSION_COMMAND_CLASS_GET_V3,
                                    COMMAND_CLASS_DOOR_LOCK};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_version_cc_extended_cc()
{
  attribute_store_node_t cc_version_node
    = attribute_store_add_node(ZWAVE_CC_VERSION_ATTRIBUTE(0xFAFA),
                               endpoint_id_node);

  // Bail out for extended CCs, consider they are version 1.
  TEST_ASSERT_NOT_NULL(version_cc_get);
  TEST_ASSERT_EQUAL(
    SL_STATUS_ALREADY_EXISTS,
    version_cc_get(cc_version_node, received_frame, &received_frame_size));
  TEST_ASSERT_EQUAL(0, received_frame_size);
  u8_value = 0;
  attribute_store_get_reported(cc_version_node, &u8_value, sizeof(u8_value));
  TEST_ASSERT_EQUAL(1, u8_value);
}

void test_zwave_command_class_version_cc_already_known()
{
  attribute_store_node_t endpoint_0_node
    = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, node_id_node);
  zwave_endpoint_id_t endpoint_0 = 0;
  attribute_store_set_reported(endpoint_0_node,
                               &endpoint_0,
                               sizeof(endpoint_0));
  attribute_store_node_t cc_version_node_1 = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_SENSOR_BINARY),
    endpoint_0_node);
  uint8_t known_version = 23;
  attribute_store_set_reported(cc_version_node_1,
                               &known_version,
                               sizeof(known_version));
  attribute_store_node_t cc_version_node_2 = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_SENSOR_BINARY),
    endpoint_id_node);

  // Bail out for extended CCs, consider they are version 1.
  TEST_ASSERT_NOT_NULL(version_cc_get);
  TEST_ASSERT_EQUAL(
    SL_STATUS_ALREADY_EXISTS,
    version_cc_get(cc_version_node_2, received_frame, &received_frame_size));
  TEST_ASSERT_EQUAL(0, received_frame_size);
  u8_value = 0;
  attribute_store_get_reported(cc_version_node_2, &u8_value, sizeof(u8_value));
  TEST_ASSERT_EQUAL(known_version, u8_value);
}

void test_zwave_command_class_version_report_happy_case_no_data()
{
  attribute_store_node_t version_report_node
    = attribute_store_add_node(ATTRIBUTE(VERSION_REPORT_DATA),
                               endpoint_id_node);

  // Simulate an incoming report, first too short:
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_VERSION_V3, VERSION_REPORT_V3};

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  u8_value = 0;
  attribute_store_get_reported(version_report_node,
                               &u8_value,
                               sizeof(u8_value));
  TEST_ASSERT_EQUAL(1, u8_value);

  // Check that 1 field is set: Default Hardware Version will be set if not found
  TEST_ASSERT_EQUAL(1,
                    attribute_store_get_node_child_count(version_report_node));
}

void test_zwave_command_class_version_report_happy_case_library()
{
  attribute_store_node_t version_report_node
    = attribute_store_add_node(ATTRIBUTE(VERSION_REPORT_DATA),
                               endpoint_id_node);

  // Simulate an incoming report with library field
  uint8_t zwave_library = 34;
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_VERSION_V3, VERSION_REPORT_V3, zwave_library, 0x00};

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  u8_value = 0;
  attribute_store_get_reported(version_report_node,
                               &u8_value,
                               sizeof(u8_value));
  TEST_ASSERT_EQUAL(1, u8_value);

  // Check that the field was picked up by the report
  TEST_ASSERT_EQUAL(2,
                    attribute_store_get_node_child_count(version_report_node));
  attribute_store_node_t library_type_node
    = attribute_store_get_node_child_by_type(version_report_node,
                                             ATTRIBUTE(ZWAVE_LIBRARY_TYPE),
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, library_type_node);
  u8_value = 0;
  attribute_store_get_reported(library_type_node, &u8_value, sizeof(u8_value));
  TEST_ASSERT_EQUAL(zwave_library, u8_value);
}

void test_zwave_command_class_version_report_happy_case_protocol_version()
{
  attribute_store_node_t version_report_node
    = attribute_store_add_node(ATTRIBUTE(VERSION_REPORT_DATA),
                               endpoint_id_node);

  // Simulate an incoming report with protocol version
  uint8_t version                 = 12;
  uint8_t sub_version             = 8;
  uint32_t zwave_protocol_version = (version << 16) | (sub_version << 8);
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_VERSION_V3, VERSION_REPORT_V3, 0x00, version, sub_version};

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  u8_value = 0;
  attribute_store_get_reported(version_report_node,
                               &u8_value,
                               sizeof(u8_value));
  TEST_ASSERT_EQUAL(1, u8_value);

  // Check that the field was picked up by the report
  TEST_ASSERT_EQUAL(3,
                    attribute_store_get_node_child_count(version_report_node));
  attribute_store_node_t protocol_version_node
    = attribute_store_get_node_child_by_type(version_report_node,
                                             ATTRIBUTE(ZWAVE_PROTOCOL_VERSION),
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, protocol_version_node);
  u32_value = 0;
  attribute_store_get_reported(protocol_version_node,
                               &u32_value,
                               sizeof(u32_value));
  TEST_ASSERT_EQUAL(zwave_protocol_version, u32_value);
}

void test_zwave_command_class_version_report_happy_case_firmware_0()
{
  attribute_store_node_t version_report_node
    = attribute_store_add_node(ATTRIBUTE(VERSION_REPORT_DATA),
                               endpoint_id_node);

  // Simulate an incoming report with protocol version
  uint8_t version             = 52;
  uint8_t sub_version         = 244;
  uint32_t firmware_0_version = (version << 16) | (sub_version << 8);
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));
  const uint8_t incoming_frame[] = {COMMAND_CLASS_VERSION_V3,
                                    VERSION_REPORT_V3,
                                    0x00,
                                    0x00,
                                    0x00,
                                    version,
                                    sub_version};

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  // Report received, set to 1
  u8_value = 0;
  attribute_store_get_reported(version_report_node,
                               &u8_value,
                               sizeof(u8_value));
  TEST_ASSERT_EQUAL(1, u8_value);

  // Check that the field was picked up by the report
  TEST_ASSERT_EQUAL(4,
                    attribute_store_get_node_child_count(version_report_node));
  uint32_t firmware_id = 0;
  attribute_store_node_t firmware_0_node
    = attribute_store_get_node_child_by_value(version_report_node,
                                              ATTRIBUTE(FIRMWARE),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&firmware_id,
                                              sizeof(firmware_id),
                                              0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, firmware_0_node);
  attribute_store_node_t firmware_0_version_node
    = attribute_store_get_node_child_by_type(firmware_0_node,
                                             ATTRIBUTE(FIRMWARE_VERSION),
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, firmware_0_version_node);
  u32_value = 0;
  attribute_store_get_reported(firmware_0_version_node,
                               &u32_value,
                               sizeof(u32_value));
  TEST_ASSERT_EQUAL(firmware_0_version, u32_value);
}

void test_zwave_command_class_version_report_hardware_version()
{
  attribute_store_node_t version_report_node
    = attribute_store_add_node(ATTRIBUTE(VERSION_REPORT_DATA),
                               endpoint_id_node);

  // Simulate an incoming report with protocol version
  uint8_t hardware_version = 10;
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));
  const uint8_t incoming_frame[] = {COMMAND_CLASS_VERSION_V3,
                                    VERSION_REPORT_V3,
                                    0x00,
                                    0x00,
                                    0x00,
                                    0x00,
                                    0x00,
                                    hardware_version};

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  // Report received, set to 1
  u8_value = 0;
  attribute_store_get_reported(version_report_node,
                               &u8_value,
                               sizeof(u8_value));
  TEST_ASSERT_EQUAL(1, u8_value);

  // Check that the field was picked up by the report
  TEST_ASSERT_EQUAL(4,
                    attribute_store_get_node_child_count(version_report_node));
  attribute_store_node_t hardware_version_node
    = attribute_store_get_node_child_by_type(version_report_node,
                                             ATTRIBUTE(HARDWARE_VERSION),
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, hardware_version_node);
  u32_value = 0;
  attribute_store_get_reported(hardware_version_node,
                               &u32_value,
                               sizeof(u32_value));
  TEST_ASSERT_EQUAL(hardware_version, u32_value);
}
void test_zwave_command_class_version_report_happy_case_multi_firmwares()
{
  attribute_store_node_t version_report_node
    = attribute_store_add_node(ATTRIBUTE(VERSION_REPORT_DATA),
                               endpoint_id_node);

  // Simulate an incoming report with protocol version
  uint8_t version             = 2;
  uint8_t sub_version         = 3;
  uint32_t firmware_2_version = (version << 16) | (sub_version << 8);
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));
  const uint8_t incoming_frame[] = {
    COMMAND_CLASS_VERSION_V3,
    VERSION_REPORT_V3,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x03,         // 3 firmware IDs
    0x00,         // Firmware ID 1
    0x00,         // Firmware ID 1
    version,      // Firmware ID 2
    sub_version,  // Firmware ID 2
    0x00,         // Firmware ID 3
    0x00          // Firmware ID 3
  };

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  // Report received, set to 1
  u8_value = 0;
  attribute_store_get_reported(version_report_node,
                               &u8_value,
                               sizeof(u8_value));
  TEST_ASSERT_EQUAL(1, u8_value);

  // Check that the field was picked up by the report
  TEST_ASSERT_EQUAL(7,
                    attribute_store_get_node_child_count(version_report_node));
  uint32_t firmware_id = 2;
  attribute_store_node_t firmware_2_node
    = attribute_store_get_node_child_by_value(version_report_node,
                                              ATTRIBUTE(FIRMWARE),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&firmware_id,
                                              sizeof(firmware_id),
                                              0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, firmware_2_node);
  attribute_store_node_t firmware_2_version_node
    = attribute_store_get_node_child_by_type(firmware_2_node,
                                             ATTRIBUTE(FIRMWARE_VERSION),
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, firmware_2_version_node);
  u32_value = 0;
  attribute_store_get_reported(firmware_2_version_node,
                               &u32_value,
                               sizeof(u32_value));
  TEST_ASSERT_EQUAL(firmware_2_version, u32_value);
}

void test_zwave_command_class_version_command_class_report_happy_case()
{
  attribute_store_node_t cc_version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_DOOR_LOCK),
    endpoint_id_node);

  // Fiddle with the endpoint in the connection info:
  connection_info.remote.endpoint_id = 23;

  // Simulate an incoming report with version cc data
  zwave_cc_version_t received_version = 34;
  const uint8_t incoming_frame[]      = {COMMAND_CLASS_VERSION_V3,
                                    VERSION_COMMAND_CLASS_REPORT_V3,
                                    COMMAND_CLASS_DOOR_LOCK,
                                    received_version};

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));
  received_version = 0;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_get_reported(cc_version_node,
                                                 &received_version,
                                                 sizeof(received_version)));
  TEST_ASSERT_EQUAL(34, received_version);
}

void test_zwave_command_class_version_command_class_report_too_short()
{
  attribute_store_node_t cc_version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_DOOR_LOCK),
    endpoint_id_node);

  const uint8_t incoming_frame[] = {COMMAND_CLASS_VERSION_V3,
                                    VERSION_COMMAND_CLASS_REPORT_V3,
                                    COMMAND_CLASS_DOOR_LOCK};

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(cc_version_node, REPORTED_ATTRIBUTE));
}

void test_zwave_command_class_version_capabilities_report_happy_case()
{
  attribute_store_node_t zwave_sw_support_node
    = attribute_store_add_node(ATTRIBUTE(ZWAVE_SOFTWARE_GET_SUPPORT),
                               endpoint_id_node);

  // Simulate an incoming report
  uint8_t support
    = VERSION_CAPABILITIES_REPORT_PROPERTIES1_Z_WAVE_SOFTWARE_BIT_MASK_V3;
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_VERSION_V3, VERSION_CAPABILITIES_REPORT_V3, support};

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  bool received_value = false;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_get_reported(zwave_sw_support_node,
                                                 &received_value,
                                                 sizeof(received_value)));
  TEST_ASSERT_TRUE(received_value);
  // A bunch of new attributes are created for the Z-Wave Software data
  TEST_ASSERT_EQUAL(8, attribute_store_get_node_child_count(endpoint_id_node));
}

void test_zwave_command_class_version_capabilities_report_too_short()
{
  attribute_store_node_t zwave_sw_support_node
    = attribute_store_add_node(ATTRIBUTE(ZWAVE_SOFTWARE_GET_SUPPORT),
                               endpoint_id_node);

  // Simulate an incoming report
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_VERSION_V3, VERSION_CAPABILITIES_REPORT_V3};

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  // Frame did not have any effect.
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(zwave_sw_support_node,
                                                     REPORTED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));
}

void test_zwave_command_class_version_software_report_too_short()
{
  attribute_store_node_t zwave_sdk_version_node
    = attribute_store_add_node(ATTRIBUTE(SDK_VERSION), endpoint_id_node);
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  uint8_t sdk_major              = 23;
  uint8_t sdk_minor              = 11;
  uint32_t sdk_version           = (sdk_major << 16) | (sdk_minor << 8);
  const uint8_t incoming_frame[] = {COMMAND_CLASS_VERSION_V3,
                                    VERSION_ZWAVE_SOFTWARE_REPORT_V3,
                                    sdk_major,
                                    sdk_minor};

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  u32_value = 0;
  attribute_store_get_reported(zwave_sdk_version_node,
                               &u32_value,
                               sizeof(u32_value));
  // Nothing should have been copied
  TEST_ASSERT_NOT_EQUAL(sdk_version, u32_value);
}

void test_zwave_command_class_version_software_report_sdk_version()
{
  attribute_store_node_t zwave_sdk_version_node
    = attribute_store_add_node(ATTRIBUTE(SDK_VERSION), endpoint_id_node);
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  uint8_t sdk_major    = 23;
  uint8_t sdk_minor    = 11;
  uint8_t sdk_patch    = 01;
  uint32_t sdk_version = (sdk_major << 16) | (sdk_minor << 8) | sdk_patch;
  const uint8_t incoming_frame[] = {
    COMMAND_CLASS_VERSION_V3,
    VERSION_ZWAVE_SOFTWARE_REPORT_V3,
    sdk_major,
    sdk_minor,
    sdk_patch,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
  };

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  u32_value = 0;
  attribute_store_get_reported(zwave_sdk_version_node,
                               &u32_value,
                               sizeof(u32_value));
  TEST_ASSERT_EQUAL(sdk_version, u32_value);
}

void test_zwave_command_class_version_software_report_app_framework_version()
{
  attribute_store_node_t app_framework_version_node
    = attribute_store_add_node(ATTRIBUTE(APPLICATION_FRAMEWORK_API_VERSION),
                               endpoint_id_node);
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  uint8_t major                  = 23;
  uint8_t minor                  = 11;
  uint8_t patch                  = 01;
  uint32_t version               = (major << 16) | (minor << 8) | patch;
  const uint8_t incoming_frame[] = {
    COMMAND_CLASS_VERSION_V3,
    VERSION_ZWAVE_SOFTWARE_REPORT_V3,
    0x00,
    0x00,
    0x00,
    major,
    minor,
    patch,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
  };

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  u32_value = 0;
  attribute_store_get_reported(app_framework_version_node,
                               &u32_value,
                               sizeof(u32_value));
  TEST_ASSERT_EQUAL(version, u32_value);
}

void test_zwave_command_class_version_software_report_app_framework_build_number()
{
  attribute_store_node_t test_node
    = attribute_store_add_node(ATTRIBUTE(APPLICATION_FRAMEWORK_BUILD_NUMBER),
                               endpoint_id_node);
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  uint8_t msb                    = 98;
  uint8_t lsb                    = 74;
  uint32_t number                = (msb << 8) | lsb;
  const uint8_t incoming_frame[] = {
    COMMAND_CLASS_VERSION_V3,
    VERSION_ZWAVE_SOFTWARE_REPORT_V3,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    msb,
    lsb,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
  };

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  u32_value = 0;
  attribute_store_get_reported(test_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(number, u32_value);
}

void test_zwave_command_class_version_software_report_host_interface_version()
{
  attribute_store_node_t test_node
    = attribute_store_add_node(ATTRIBUTE(HOST_INTERFACE_VERSION),
                               endpoint_id_node);

  uint8_t major                  = 23;
  uint8_t minor                  = 11;
  uint8_t patch                  = 01;
  uint32_t version               = (major << 16) | (minor << 8) | patch;
  const uint8_t incoming_frame[] = {
    COMMAND_CLASS_VERSION_V3,
    VERSION_ZWAVE_SOFTWARE_REPORT_V3,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    major,
    minor,
    patch,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
  };

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  u32_value = 0;
  attribute_store_get_reported(test_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(version, u32_value);
}

void test_zwave_command_class_version_software_report_build_number()
{
  attribute_store_node_t test_node
    = attribute_store_add_node(ATTRIBUTE(HOST_INTERFACE_BUILD_NUMBER),
                               endpoint_id_node);

  uint8_t msb                    = 98;
  uint8_t lsb                    = 74;
  uint32_t number                = (msb << 8) | lsb;
  const uint8_t incoming_frame[] = {
    COMMAND_CLASS_VERSION_V3,
    VERSION_ZWAVE_SOFTWARE_REPORT_V3,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    msb,
    lsb,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
  };

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  u32_value = 0;
  attribute_store_get_reported(test_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(number, u32_value);
}

void test_zwave_command_class_version_software_report_zwave_protocol_version()
{
  attribute_store_node_t version_report_node
    = attribute_store_add_node(ATTRIBUTE(VERSION_REPORT_DATA),
                               endpoint_id_node);

  uint8_t major                  = 23;
  uint8_t minor                  = 11;
  uint8_t patch                  = 01;
  uint32_t version               = (major << 16) | (minor << 8) | patch;
  const uint8_t incoming_frame[] = {
    COMMAND_CLASS_VERSION_V3,
    VERSION_ZWAVE_SOFTWARE_REPORT_V3,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    major,
    minor,
    patch,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
  };

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  // Check that the field was picked up by the report
  TEST_ASSERT_EQUAL(2,
                    attribute_store_get_node_child_count(version_report_node));
  attribute_store_node_t protocol_version_node
    = attribute_store_get_node_child_by_type(version_report_node,
                                             ATTRIBUTE(ZWAVE_PROTOCOL_VERSION),
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, protocol_version_node);
  u32_value = 0;
  attribute_store_get_reported(protocol_version_node,
                               &u32_value,
                               sizeof(u32_value));
  TEST_ASSERT_EQUAL(version, u32_value);
}

void test_zwave_command_class_version_software_report_protocol_build_number()
{
  attribute_store_node_t test_node
    = attribute_store_add_node(ATTRIBUTE(ZWAVE_PROTOCOL_BUILD_NUMBER),
                               endpoint_id_node);

  uint8_t msb                    = 98;
  uint8_t lsb                    = 74;
  uint32_t number                = (msb << 8) | lsb;
  const uint8_t incoming_frame[] = {
    COMMAND_CLASS_VERSION_V3,
    VERSION_ZWAVE_SOFTWARE_REPORT_V3,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    msb,
    lsb,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
  };

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  u32_value = 0;
  attribute_store_get_reported(test_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(number, u32_value);
}

void test_zwave_command_class_version_software_report_application_version()
{
  attribute_store_node_t version_report_node
    = attribute_store_add_node(ATTRIBUTE(VERSION_REPORT_DATA),
                               endpoint_id_node);

  uint8_t major                  = 23;
  uint8_t minor                  = 11;
  uint8_t patch                  = 01;
  uint32_t version               = (major << 16) | (minor << 8) | patch;
  const uint8_t incoming_frame[] = {
    COMMAND_CLASS_VERSION_V3,
    VERSION_ZWAVE_SOFTWARE_REPORT_V3,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    major,
    minor,
    patch,
    0x00,
    0x00,
  };

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  // Check that the field was picked up by the report
  uint32_t firmware_id = 0;
  attribute_store_node_t firmware_0_node
    = attribute_store_get_node_child_by_value(version_report_node,
                                              ATTRIBUTE(FIRMWARE),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&firmware_id,
                                              sizeof(firmware_id),
                                              0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, firmware_0_node);
  attribute_store_node_t firmware_0_version_node
    = attribute_store_get_node_child_by_type(firmware_0_node,
                                             ATTRIBUTE(FIRMWARE_VERSION),
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, firmware_0_version_node);
  u32_value = 0;
  attribute_store_get_reported(firmware_0_version_node,
                               &u32_value,
                               sizeof(u32_value));
  TEST_ASSERT_EQUAL(version, u32_value);
}

void test_zwave_command_class_version_software_report_application_build_number()
{
  attribute_store_node_t test_node
    = attribute_store_add_node(ATTRIBUTE(APPLICATION_BUILD_NUMBER),
                               endpoint_id_node);

  uint8_t msb                    = 98;
  uint8_t lsb                    = 74;
  uint32_t number                = (msb << 8) | lsb;
  const uint8_t incoming_frame[] = {
    COMMAND_CLASS_VERSION_V3,
    VERSION_ZWAVE_SOFTWARE_REPORT_V3,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    msb,
    lsb,
  };

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    version_handler.support_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    version_handler.control_handler(&connection_info,
                                                    incoming_frame,
                                                    sizeof(incoming_frame)));

  u32_value = 0;
  attribute_store_get_reported(test_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(number, u32_value);
}
