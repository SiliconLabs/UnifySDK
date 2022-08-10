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
#include "zigpc_discovery_callbacks.hpp"

extern "C" {

// Shared Unify includes
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
  zigpc_discovery::callbacks::clear();
}

/**
 * Teardown after each test case
 *
 */
void tearDown(void) {}

void test_device_specific_update(void)
{
  zigbee_eui64_uint_t eui64               = 0x3424304AB32DC0B2;
  zigpc_discovery_status discovery_status = DISCOVERY_SUCCESS;

  // ARRANGE
  test_discovery_status_callback_Expect(eui64, discovery_status);

  // ACT
  sl_status_t add_status
    = zigpc_discovery::callbacks::add(eui64, test_discovery_status_callback);

  zigpc_discovery::callbacks::notify(eui64, discovery_status);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, add_status);
}

void test_all_device_discovery_updates(void)
{
  zigbee_eui64_uint_t eui64               = 0x012404AB32DC00F8;
  zigpc_discovery_status discovery_status = DISCOVERY_SUCCESS;

  // ARRANGE
  test_discovery_status_listener_Expect(eui64, discovery_status);

  // ACT
  sl_status_t add_status = zigpc_discovery::callbacks::add(
    zigpc_discovery::callbacks::WILDCARD_EUI64,
    test_discovery_status_listener);

  zigpc_discovery::callbacks::notify(eui64, discovery_status);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, add_status);
}

void test_adding_listener_and_callback_to_be_notfied(void)
{
  zigbee_eui64_uint_t eui64_1          = 0x0124099B32DC0048;
  zigbee_eui64_uint_t eui64_2          = 0x4B2893276591FBAC;
  zigpc_discovery_status disc_status_1 = DISCOVERY_SUCCESS;
  zigpc_discovery_status disc_status_2 = DISCOVERY_START;

  // ARRANGE
  test_discovery_status_listener_Expect(eui64_1, disc_status_1);
  test_discovery_status_listener_Expect(eui64_2, disc_status_2);

  test_discovery_status_callback_Expect(eui64_1, disc_status_1);

  // ACT
  sl_status_t add_listener_status = zigpc_discovery::callbacks::add(
    zigpc_discovery::callbacks::WILDCARD_EUI64,
    test_discovery_status_listener);
  sl_status_t add_callback_status
    = zigpc_discovery::callbacks::add(eui64_1, test_discovery_status_callback);

  zigpc_discovery::callbacks::notify(eui64_1, disc_status_1);
  zigpc_discovery::callbacks::notify(eui64_2, disc_status_2);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, add_listener_status);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, add_callback_status);
}

void test_removing_device_specific_callback(void)
{
  zigbee_eui64_uint_t eui64               = 0xFF242C4200DC00F8;
  zigpc_discovery_status discovery_status = DEVICE_DISCOVERY_FAIL;

  // ARRANGE

  // ACT
  sl_status_t add_status
    = zigpc_discovery::callbacks::add(eui64, test_discovery_status_callback);
  sl_status_t remove_status
    = zigpc_discovery::callbacks::remove(eui64, test_discovery_status_callback);

  zigpc_discovery::callbacks::notify(eui64, discovery_status);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, add_status);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, remove_status);
}

void test_removing_all_device_listener(void)
{
  zigbee_eui64_uint_t eui64               = 0xAF24234E00DC00F8;
  zigpc_discovery_status discovery_status = ENDPOINT_DISCOVERY_FAIL;

  // ARRANGE

  // ACT
  sl_status_t add_status = zigpc_discovery::callbacks::add(
    zigpc_discovery::callbacks::WILDCARD_EUI64,
    test_discovery_status_listener);

  sl_status_t remove_status = zigpc_discovery::callbacks::remove(
    zigpc_discovery::callbacks::WILDCARD_EUI64,
    test_discovery_status_listener);

  zigpc_discovery::callbacks::notify(eui64, discovery_status);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, add_status);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, remove_status);
}

}  // extern "C"
