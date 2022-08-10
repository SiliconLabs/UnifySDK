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
#include "zwave_transports_fixt.h"
#include "unity.h"

// Mocks
#include "zwave_s0_transport_mock.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_zwave_transports_fixt_test()
{
  // Just run init.
  zwave_s0_transport_init_ExpectAndReturn(SL_STATUS_OK);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_transports_init());

  // Run another init that goes wrong
  zwave_s0_transport_init_ExpectAndReturn(SL_STATUS_FAIL);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, zwave_transports_init());
}