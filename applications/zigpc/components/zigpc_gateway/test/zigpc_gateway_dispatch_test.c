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
#include <string.h>

#include "unity.h"

#include "zigpc_gateway_int.h"

#define TEST_UNKNOWN_NAME "UNKNOWN"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp() {}

void tearDown() {}

/**
 * @brief Verify all known states are covered
 * 
 */
void test_zigpc_gateway_dispatch_name_should_print_all_known_states(void)
{
  for (uint8_t i = (ZIGPC_GW_DISPATCH_MIN_VAL + 1);
       i < ZIGPC_GW_DISPATCH_MAX_VAL;
       i++) {
    TEST_ASSERT_TRUE(strstr(zigpc_gateway_dispatch_name(i), TEST_UNKNOWN_NAME)
                     == NULL);
  }
}

/**
 * @brief Verify unknown state print
 * 
 */
void test_zigpc_gateway_dispatch_name_should_print_unkown_states(void)
{
  TEST_ASSERT_FALSE(
    strstr(zigpc_gateway_dispatch_name(ZIGPC_GW_DISPATCH_MIN_VAL),
           TEST_UNKNOWN_NAME)
    == NULL);
  TEST_ASSERT_FALSE(
    strstr(zigpc_gateway_dispatch_name(ZIGPC_GW_DISPATCH_MAX_VAL),
           TEST_UNKNOWN_NAME)
    == NULL);
  TEST_ASSERT_FALSE(strstr(zigpc_gateway_dispatch_name(0xFF), TEST_UNKNOWN_NAME)
                    == NULL);
}
