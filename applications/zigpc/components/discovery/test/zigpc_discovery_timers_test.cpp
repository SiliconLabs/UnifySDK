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

// Component includes
#include "zigpc_discovery_timers.hpp"

extern "C" {

// Shared UIC includes
#include <unity.h>
#include <contiki_test_helper.h>

// Mocks
#include "zigpc_discovery_test_callbacks_mock.h"

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
void setUp(void)
{
  zigpc_discovery::timers::clear();
  contiki_test_helper_init();
  contiki_test_helper_run_once(0);
}

/**
 * Teardown after each test case
 *
 */
void tearDown(void)
{
  contiki_test_helper_run_once(0);
}

void test_adding_single_callback(void)
{
  zigbee_eui64_uint_t eui64               = 0x89423B42398D2548;
  clock_time_t timeout_ms                 = 100;
  zigpc_discovery_status_t timeout_status = DISCOVERY_SUCCESS;

  // ARRANGE

  // ACT
  sl_status_t add_status
    = zigpc_discovery::timers::create(eui64,
                                      timeout_ms,
                                      timeout_status,
                                      test_discovery_status_callback);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, add_status);
}

void test_invoking_single_callback(void)
{
  zigbee_eui64_uint_t eui64               = 0x2138721749DAB842;
  clock_time_t timeout_ms                 = 150;
  zigpc_discovery_status_t timeout_status = DISCOVERY_SUCCESS;

  // ARRANGE
  test_discovery_status_callback_Expect(eui64, timeout_status);

  // ACT
  sl_status_t add_status
    = zigpc_discovery::timers::create(eui64,
                                      timeout_ms,
                                      timeout_status,
                                      test_discovery_status_callback);

  contiki_test_helper_run_once(timeout_ms);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, add_status);
}

void test_adding_multiple_timers(void)
{
  zigbee_eui64_uint_t eui64_1               = 0x2138721749DAB842;
  zigbee_eui64_uint_t eui64_2               = 0xAB3E7219ACD2932F;
  clock_time_t timeout_ms_1                 = 50;
  clock_time_t timeout_ms_2                 = 150;
  zigpc_discovery_status_t timeout_status_1 = DISCOVERY_SUCCESS;
  zigpc_discovery_status_t timeout_status_2 = ENDPOINT_DISCOVERY_FAIL;

  // ARRANGE 1
  // ACT 1
  sl_status_t add_status_1
    = zigpc_discovery::timers::create(eui64_1,
                                      timeout_ms_1,
                                      timeout_status_1,
                                      test_discovery_status_callback);
  sl_status_t add_status_2
    = zigpc_discovery::timers::create(eui64_2,
                                      timeout_ms_2,
                                      timeout_status_2,
                                      test_discovery_status_callback);
  // ASSERT 1
  TEST_ASSERT_EQUAL(SL_STATUS_OK, add_status_1);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, add_status_2);

  // ARRANGE 2
  test_discovery_status_callback_Expect(eui64_1, timeout_status_1);
  // ACT 2
  contiki_test_helper_run_once(timeout_ms_1);
  // ASSERT 2

  // ARRANGE 3
  test_discovery_status_callback_Expect(eui64_2, timeout_status_2);
  // ACT 3
  contiki_test_helper_run_once(timeout_ms_2 - timeout_ms_1);
  // ASSERT 3
}

void test_removing_timer_before_expiry(void)
{
  zigbee_eui64_uint_t eui64               = 0xE7B2034278CD7922;
  clock_time_t timeout_ms                 = 100;
  zigpc_discovery_status_t timeout_status = DEVICE_DISCOVERY_FAIL;

  // ARRANGE
  // ACT
  sl_status_t add_status
    = zigpc_discovery::timers::create(eui64,
                                      timeout_ms,
                                      timeout_status,
                                      test_discovery_status_callback);
  contiki_test_helper_run_once(timeout_ms - 1);

  sl_status_t remove_status = zigpc_discovery::timers::remove(eui64);
  contiki_test_helper_run_once(timeout_ms);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, add_status);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, remove_status);
}

}  // extern "C"
