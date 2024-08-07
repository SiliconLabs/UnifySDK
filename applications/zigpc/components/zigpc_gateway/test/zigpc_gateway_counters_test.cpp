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

void test_clear_counters_sanity(void)
{
  // ARRANGE
  zigbeeHostClearCounters_Expect();

  // ACT
  zigpc_gateway_clear_counters();

  // ASSERT (Handled by CMock)
}

void test_get_counters_capacity_sanity(void)
{
  // ARRANGE

  // ACT
  size_t size = zigpc_gateway_get_counters_capacity();

  // ASSERT (Handled by CMock)
  TEST_ASSERT_EQUAL(SL_ZIGBEE_COUNTER_TYPE_COUNT, size);
}

void test_get_counters_list_invalid_args(void)
{
  uint16_t list[2];

  // ARRANGE

  // ACT
  sl_status_t status_np = zigpc_gateway_get_counters_list(NULL, 0);
  sl_status_t status_overflow
    = zigpc_gateway_get_counters_list(list, SL_ZIGBEE_COUNTER_TYPE_COUNT + 1);
  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, status_np);
  TEST_ASSERT_EQUAL(SL_STATUS_WOULD_OVERFLOW, status_overflow);
}

void test_get_counters_list_less_than_max(void)
{
  size_t count = 2U;
  uint16_t list[count];
  uint16_t expected_list[count] = {0x1, 0x2};
  // ARRANGE

  zigbeeHostGetCountersList_ExpectAndReturn(nullptr, 2, SL_STATUS_OK );
  zigbeeHostGetCountersList_IgnoreArg_list();
  zigbeeHostGetCountersList_ReturnMemThruPtr_list(expected_list,
                                                  sizeof(uint16_t) * count);
  sl_status_t status = zigpc_gateway_get_counters_list(list, count);

  // ASSERT (Handled by CMock)
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_HEX16_ARRAY(expected_list, list, count);
}

void test_get_counters_list_sanity(void)
{
  uint16_t list[SL_ZIGBEE_COUNTER_TYPE_COUNT] = {};
  uint16_t expected_list[SL_ZIGBEE_COUNTER_TYPE_COUNT];

  // ARRANGE
  zigbeeHostGetCountersList_ExpectAndReturn(nullptr,
                                            SL_ZIGBEE_COUNTER_TYPE_COUNT,
                                            SL_STATUS_OK );
  zigbeeHostGetCountersList_IgnoreArg_list();
  zigbeeHostGetCountersList_ReturnMemThruPtr_list(expected_list,
                                                  sizeof(uint16_t)
                                                    * SL_ZIGBEE_COUNTER_TYPE_COUNT);

  // ACT
  sl_status_t status
    = zigpc_gateway_get_counters_list(list, SL_ZIGBEE_COUNTER_TYPE_COUNT);

  // ASSERT (Handled by CMock)
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_UINT16_ARRAY(expected_list, list, SL_ZIGBEE_COUNTER_TYPE_COUNT);
}

void test_get_counters_entry_label_sanity(void)
{
  // ARRANGE

  // ACT
  const char *entry_invalid = zigpc_gateway_get_counters_entry_label(0xFFFF);
  const char *entry_valid
    = zigpc_gateway_get_counters_entry_label(SL_ZIGBEE_COUNTER_NEIGHBOR_ADDED);

  // ASSERT
  TEST_ASSERT_NULL(entry_invalid);
  TEST_ASSERT_EQUAL_STRING("NeighborAdded", entry_valid);
}

} /* extern C */
