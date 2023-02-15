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

extern "C" {

// Shared Unify includes
#include <unity.h>

// ZigPC includes
#include "zigbee_host_mock.h"

// Component includes
#include "zigpc_gateway.h"
#include "zigpc_gateway_int.h"

/**
 * Setup the test suite (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp(void) {}

/**
 * Teardown the test suite (called once after all test_xxx functions are called)
 *
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/**
 * Setup before each test case
 *
 */
void setUp(void) {}

/**
 * Teardown after each test case
 *
 */
void tearDown(void) {}

void test_get_neighbor_count_sanity(void)
{
  // ARRANGE
  zigbeeHostGetNeighborCount_ExpectAndReturn(42);

  // ACT
  uint8_t neighbor_count = zigpc_gateway_get_neighbor_count();

  // ASSERT
  TEST_ASSERT_EQUAL(neighbor_count, 42);
}

void test_zigpc_gateway_get_neighbor_eui64_sanity()
{
  // ARRANGE
  uint8_t mock_neighbor_eui64[] = {1, 2, 3, 4, 5, 6, 7, 8};
  uint8_t eui_64_array[8];

  zigbeeHostGetNeighborEUI64_ExpectAndReturn(0, NULL, SL_STATUS_OK);
  zigbeeHostGetNeighborEUI64_IgnoreArg_eui64();
  zigbeeHostGetNeighborEUI64_ReturnArrayThruPtr_eui64(mock_neighbor_eui64, 8);
  // ACT
  sl_status_t result = zigpc_gateway_get_neighbor_eui64(0, eui_64_array);
  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, result);
}

void test_zigpc_gateway_get_neighbor_eui64_invalid_pointer_as_input()
{
  // ARRANGE
  uint8_t* eui_64_array_null    = NULL;

  // ACT
  sl_status_t result = zigpc_gateway_get_neighbor_eui64(0, eui_64_array_null);
  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, result);
}
}