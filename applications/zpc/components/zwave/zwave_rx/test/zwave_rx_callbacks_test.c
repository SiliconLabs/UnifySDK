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
#include "zwave_rx_callbacks.h"
#include "zwave_rx_zwapi_callbacks.h"

// Mocks
#include "zwapi_protocol_basis_mock.h"

static int callback_1_counter;
static int callback_2_counter;
static int callback_3_counter;

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
  callback_1_counter = 0;
  callback_2_counter = 0;
  callback_3_counter = 0;
}

static void test_callback_1()
{
  callback_1_counter += 1;
}

static void test_callback_2()
{
  callback_2_counter += 1;
}

static void test_callback_3()
{
  callback_3_counter += 1;
}

void test_zwave_rx_callbacks_test()
{
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_rx_register_zwave_api_started_callback(&test_callback_1));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_rx_register_zwave_api_started_callback(&test_callback_1));

  TEST_ASSERT_EQUAL(0, callback_1_counter);
  TEST_ASSERT_EQUAL(0, callback_2_counter);
  TEST_ASSERT_EQUAL(0, callback_3_counter);

  // Simulate the Z-Wave API started received
  zwapi_set_node_id_basetype_ExpectAndReturn(NODEID_16BITS, SL_STATUS_OK);
  zwave_rx_zwave_api_started(NULL, 0);

  // Callback 1 should have been called
  TEST_ASSERT_EQUAL(1, callback_1_counter);
  TEST_ASSERT_EQUAL(0, callback_2_counter);
  TEST_ASSERT_EQUAL(0, callback_3_counter);

  // Try to register a NULL pointer:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_rx_register_zwave_api_started_callback(NULL));

  // Simulate the Z-Wave API started received
  zwapi_set_node_id_basetype_ExpectAndReturn(NODEID_16BITS, SL_STATUS_OK);
  zwave_rx_zwave_api_started(NULL, 0);

  // Callback 1 should have been called
  TEST_ASSERT_EQUAL(2, callback_1_counter);
  TEST_ASSERT_EQUAL(0, callback_2_counter);
  TEST_ASSERT_EQUAL(0, callback_3_counter);

  // Add a new callback:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_rx_register_zwave_api_started_callback(&test_callback_2));

  // Simulate the Z-Wave API started received
  zwapi_set_node_id_basetype_ExpectAndReturn(NODEID_16BITS, SL_STATUS_OK);
  zwave_rx_zwave_api_started(NULL, 0);

  // Callback 1 amnd 2 should have been called
  TEST_ASSERT_EQUAL(3, callback_1_counter);
  TEST_ASSERT_EQUAL(1, callback_2_counter);
  TEST_ASSERT_EQUAL(0, callback_3_counter);

  // Add a new callback, no more space:
  TEST_ASSERT_EQUAL(
    SL_STATUS_NO_MORE_RESOURCE,
    zwave_rx_register_zwave_api_started_callback(&test_callback_3));

  // Simulate the Z-Wave API started received
  zwapi_set_node_id_basetype_ExpectAndReturn(NODEID_16BITS, SL_STATUS_OK);
  zwave_rx_zwave_api_started(NULL, 0);

  // Callback 1 amnd 2 should have been called
  TEST_ASSERT_EQUAL(4, callback_1_counter);
  TEST_ASSERT_EQUAL(2, callback_2_counter);
  TEST_ASSERT_EQUAL(0, callback_3_counter);
}