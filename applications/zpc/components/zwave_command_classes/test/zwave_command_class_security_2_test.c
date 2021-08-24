/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
// File being tested
#include "zwave_command_class_security_2.h"

// Generic includes
#include "unity.h"
#include <string.h>

// Includes from other components
#include "ZW_classcmd.h"
#include "zwave_tx_scheme_selector_mock.h"
#include "zwave_command_handler_mock.h"
#include "zwave_controller_keyset_mock.h"
#include "zwave_controller_connection_info.h"

#include "attribute_resolver_mock.h"
#include "attribute_store_mock.h"
#include "zpc_attribute_store_network_helper_mock.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute_store_helper_mock.h"
#include "zwave_unid_mock.h"

// Static test variables
static zwave_controller_connection_info_t test_connection_info;
static zwave_command_handler_t security_2_handler = {};
static uint8_t test_frame_data[]                  = {0x9F, 0x0D, 0x5E, 0x61};

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  security_2_handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    security_2_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_2, security_2_handler.command_class);
  TEST_ASSERT_EQUAL(SECURITY_2_VERSION, security_2_handler.version);
  TEST_ASSERT_NOT_NULL(security_2_handler.control_handler);
  TEST_ASSERT_NOT_NULL(security_2_handler.support_handler);
  TEST_ASSERT_TRUE(security_2_handler.manual_security_validation);

  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  memset(&test_connection_info, 0, sizeof(zwave_controller_connection_info_t));

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  // Call init
  zwave_command_class_security_2_init();
}

void test_zwave_command_class_Security_2_Commands_Supported_Report()
{
  // Send unsupported command to s2 CC handler
  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    security_2_handler.control_handler(NULL,
                                       test_frame_data,
                                       sizeof(test_frame_data)));

  // Send Security 2 Commands Supported Report command to handler
  test_frame_data[1]                  = SECURITY_2_COMMANDS_SUPPORTED_REPORT;
  test_connection_info.remote.node_id = 23;
  test_connection_info.remote.endpoint_id = 2;

  unid_t test_unid                            = "zw99999";
  attribute_store_node_t endpoint_node        = 0x3555;
  attribute_store_node_t test_secure_nif_node = 0x7778;

  zwave_unid_from_node_id_Expect(test_connection_info.remote.node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_connection_info.remote.endpoint_id,
    endpoint_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_ZWAVE_SECURE_NIF,
    0,
    test_secure_nif_node);

  zwave_tx_scheme_get_node_highest_security_class_ExpectAndReturn(
    test_connection_info.remote.node_id,
    SL_STATUS_OK);

  attribute_store_set_node_attribute_value_ExpectAndReturn(test_secure_nif_node,
                                                           REPORTED_ATTRIBUTE,
                                                           &test_frame_data[2],
                                                           2,
                                                           SL_STATUS_OK);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    security_2_handler.control_handler(&test_connection_info,
                                                       test_frame_data,
                                                       4));
}

/**
 * @brief Test S2 command supported with empty payload
 *
 * Expect handler to return without changing the attribute
 * ATTRIBUTE_ZWAVE_SECURE_NIF due to security scheme lower than zpc highest
 * security scheme.
 *
 */
void test_zwave_command_class_Security_2_Commands_Supported_ReportEmptyPayload_lower_security_scheme()
{
  unid_t test_unid                            = "zw99999";
  attribute_store_node_t endpoint_node        = 0x3555;
  attribute_store_node_t test_secure_nif_node = 0x7778;
  test_connection_info.encapsulation      = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  test_connection_info.remote.node_id     = 23;
  test_connection_info.remote.endpoint_id = 2;
  zwave_keyset_t zpc_keyset               = 0x87;

  zwave_unid_from_node_id_Expect(test_connection_info.remote.node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_connection_info.remote.endpoint_id,
    endpoint_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_ZWAVE_SECURE_NIF,
    0,
    test_secure_nif_node);

  zwave_tx_scheme_get_zpc_granted_keys_Expect(NULL);
  zwave_tx_scheme_get_zpc_granted_keys_IgnoreArg_keys();
  zwave_tx_scheme_get_zpc_granted_keys_ReturnThruPtr_keys(&zpc_keyset);
  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    zpc_keyset,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS);
  zwave_controller_encapsulation_scheme_greater_equal_ExpectAndReturn(
    ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS,
    false);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    security_2_handler.control_handler(&test_connection_info,
                                                       test_frame_data,
                                                       2));
}

/**
 * @brief Test S2 command supported with empty payload
 *
 * Expect handler delete the ATTRIBUTE_ZWAVE_SECURE_NIF
 *
 */
void test_zwave_command_class_Security_2_Commands_Supported_ReportEmptyPayload_security_scheme_equal_to_zpc()
{
  unid_t test_unid                            = "zw99999";
  attribute_store_node_t endpoint_node        = 0x3555;
  attribute_store_node_t test_secure_nif_node = 0x7778;
  test_connection_info.encapsulation
    = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS;
  test_connection_info.remote.node_id     = 23;
  test_connection_info.remote.endpoint_id = 2;
  zwave_keyset_t zpc_keyset               = 0x87;

  zwave_unid_from_node_id_Expect(test_connection_info.remote.node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_connection_info.remote.endpoint_id,
    endpoint_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_ZWAVE_SECURE_NIF,
    0,
    test_secure_nif_node);

  zwave_tx_scheme_get_zpc_granted_keys_Expect(NULL);
  zwave_tx_scheme_get_zpc_granted_keys_IgnoreArg_keys();
  zwave_tx_scheme_get_zpc_granted_keys_ReturnThruPtr_keys(&zpc_keyset);
  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    zpc_keyset,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS);
  zwave_controller_encapsulation_scheme_greater_equal_ExpectAndReturn(
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS,
    true);

  attribute_store_delete_node_ExpectAndReturn(test_secure_nif_node,
                                              SL_STATUS_OK);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    security_2_handler.control_handler(&test_connection_info,
                                                       test_frame_data,
                                                       2));
}

void test_zwave_command_class_security_2_commands_supported_get()
{
  uint8_t frame[10];
  uint16_t frame_len = 0;

  const uint8_t expected__security_2_commands_supported_get[]
    = {COMMAND_CLASS_SECURITY_2, SECURITY_2_COMMANDS_SUPPORTED_GET};

  // Call the function to test
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_command_class_security_2_commands_supported_get(frame, &frame_len));

  // There should have been 1 Get Command queued
  TEST_ASSERT_EQUAL(sizeof(ZW_SECURITY_2_COMMANDS_SUPPORTED_GET_FRAME),
                    frame_len);
  // We can only verify the payload of the command
  TEST_ASSERT_EQUAL_UINT8_ARRAY(
    expected__security_2_commands_supported_get,
    frame,
    sizeof(ZW_SECURITY_2_COMMANDS_SUPPORTED_GET_FRAME));
}
