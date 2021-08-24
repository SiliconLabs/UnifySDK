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

// Includes from this component
#include "zwave_tx_scheme_selector.h"

// Includes from other components
#include "sl_log.h"
#include "attribute_store_mock.h"
#include "attribute_store_helper_mock.h"
#include "zpc_attribute_store_mock.h"
#include "zpc_attribute_store_network_helper_mock.h"
#include "zwave_unid_mock.h"
#include "zwave_controller_keyset_mock.h"
#include "zwave_controller_connection_info.h"
#include "zwave_network_management_mock.h"
#include "attribute_store_defined_attribute_types.h"

#define LOG_TAG "zwave_tx_scheme_selector_test"

// Test includes
#include "unity.h"

static zwave_node_id_t test_node_id     = 0x05;
static uint8_t test_endpoint_id = 0x09;
static zwave_keyset_t test_keys = 0x65;
static zwave_controller_encapsulation_scheme_t test_encapsulation
  = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_zwave_tx_scheme_get_node_connection_info()
{
  static zwave_controller_connection_info_t test_connection_info = {};
  test_connection_info.remote.node_id                            = 0x05;
  test_connection_info.remote.endpoint_id                        = 0x09;
  test_connection_info.encapsulation                             = test_keys;
  unid_t test_unid                                               = "zw-000005";

  attribute_store_node_t test_home_id_node     = 0x011;
  attribute_store_node_t test_node_id_node     = 0x015;
  attribute_store_node_t test_granted_key_node = 0x018;

  zwave_unid_from_node_id_Expect(test_connection_info.remote.node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_home_id_node_ExpectAndReturn(
    test_unid,
    test_home_id_node);

  attribute_store_get_node_child_by_value_ExpectAndReturn(test_home_id_node,
                                                          ATTRIBUTE_NODE_ID,
                                                          REPORTED_ATTRIBUTE,
                                                          (uint8_t *)&test_node_id,
                                                          sizeof(test_node_id),
                                                          0,
                                                          test_node_id_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_GRANTED_SECURITY_KEYS,
    0,
    test_granted_key_node);

  attribute_store_read_value_ExpectAndReturn(test_granted_key_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(zwave_keyset_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(&test_keys,
                                                         sizeof(test_keys));

  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    test_keys,
    test_encapsulation);

  zwave_tx_scheme_get_node_connection_info(test_node_id,
                                           test_endpoint_id,
                                           &test_connection_info);
}

void test_zwave_tx_scheme_get_zpc_granted_keys()
{
  static zwave_keyset_t my_test_keys         = 0x66;
  attribute_store_node_t my_node_id_node     = 0x0ED;
  attribute_store_node_t my_granted_key_node = 0x0EE;

  get_zpc_node_id_node_ExpectAndReturn(my_node_id_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    my_node_id_node,
    ATTRIBUTE_GRANTED_SECURITY_KEYS,
    0,
    my_granted_key_node);

  attribute_store_get_reported_ExpectAndReturn(my_granted_key_node,
                                               NULL,
                                               sizeof(zwave_keyset_t),
                                               SL_STATUS_OK);
  attribute_store_get_reported_IgnoreArg_value();
  attribute_store_get_reported_ReturnMemThruPtr_value(
    &my_test_keys,
    sizeof(my_test_keys));

  zwave_tx_scheme_get_zpc_granted_keys(&my_test_keys);
}

void test_zwave_tx_scheme_get_node_tx_options()
{
  zwave_tx_options_t test_tx_options = {};
  uint32_t test_qos_priority         = 34;
  uint8_t test_number_of_response    = 34;
  uint32_t test_discard_timeout_ms   = 3;

  zwave_tx_scheme_get_node_tx_options(test_qos_priority,
                                      test_number_of_response,
                                      test_discard_timeout_ms,
                                      &test_tx_options);

  // Test our parameters
  TEST_ASSERT_EQUAL(test_qos_priority, test_tx_options.qos_priority);
  TEST_ASSERT_EQUAL(test_number_of_response,
                    test_tx_options.number_of_responses);
  TEST_ASSERT_EQUAL(test_discard_timeout_ms,
                    test_tx_options.discard_timeout_ms);

  // Test the automatic parameters
  TEST_ASSERT_EQUAL(false, test_tx_options.valid_parent_session_id);
  TEST_ASSERT_EQUAL(false, test_tx_options.use_parent_frame_options);
  TEST_ASSERT_EQUAL(false, test_tx_options.is_test_frame);
  TEST_ASSERT_EQUAL(false, test_tx_options.fasttrack);
  TEST_ASSERT_EQUAL(0, test_tx_options.rf_power);
}
