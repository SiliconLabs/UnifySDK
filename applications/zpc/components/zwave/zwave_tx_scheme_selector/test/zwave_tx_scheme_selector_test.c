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
#include "zwave_controller_storage_mock.h"
#include "zwave_network_management_mock.h"
#include "attribute_store_defined_attribute_types.h"

#define LOG_TAG "zwave_tx_scheme_selector_test"

// Test includes
#include "unity.h"

static zwave_node_id_t test_node_id = 0x05;
static uint8_t test_endpoint_id     = 0x09;
static zwave_keyset_t return_keys   = 0x00;

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
  test_connection_info.encapsulation                             = 0xFF;

  return_keys = 0x81;
  zwave_controller_storage_get_node_granted_keys_ExpectAndReturn(
    test_connection_info.remote.node_id,
    0,
    SL_STATUS_OK);
  zwave_controller_storage_get_node_granted_keys_IgnoreArg_keys();
  zwave_controller_storage_get_node_granted_keys_ReturnThruPtr_keys(
    &return_keys);

  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    0x81,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED);

  zwave_tx_scheme_get_node_connection_info(test_node_id,
                                           test_endpoint_id,
                                           &test_connection_info);

  TEST_ASSERT_EQUAL(0x5, test_node_id);
  TEST_ASSERT_EQUAL(test_connection_info.encapsulation,
                    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED);
}

void test_zwave_tx_scheme_get_zpc_granted_keys()
{
  static zwave_keyset_t my_test_keys = 0x66;
  return_keys                        = 0x87;
  zwave_network_management_get_node_id_ExpectAndReturn(1);

  zwave_controller_storage_get_node_granted_keys_ExpectAndReturn(0x01,
                                                                 0x00,
                                                                 SL_STATUS_OK);
  zwave_controller_storage_get_node_granted_keys_IgnoreArg_keys();
  zwave_controller_storage_get_node_granted_keys_ReturnThruPtr_keys(
    &return_keys);

  zwave_tx_scheme_get_zpc_granted_keys(&my_test_keys);
  TEST_ASSERT_EQUAL(my_test_keys, return_keys);
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
  TEST_ASSERT_EQUAL(false, test_tx_options.transport.valid_parent_session_id);
  TEST_ASSERT_EQUAL(false, test_tx_options.transport.is_test_frame);
  TEST_ASSERT_EQUAL(0, test_tx_options.transport.rf_power);
  TEST_ASSERT_EQUAL(false, test_tx_options.fasttrack);
}
