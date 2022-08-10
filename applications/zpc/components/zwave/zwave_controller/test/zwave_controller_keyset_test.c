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
#include "zwave_controller_keyset.h"
#include "unity.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_zwave_controller_get_key_from_encapsulation()
{
  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_S2_ACCESS_KEY,
                    zwave_controller_get_key_from_encapsulation(
                      ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS));

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY,
                    zwave_controller_get_key_from_encapsulation(
                      ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED));

  TEST_ASSERT_EQUAL(
    ZWAVE_CONTROLLER_S2_UNAUTHENTICATED_KEY,
    zwave_controller_get_key_from_encapsulation(
      ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED));

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_S0_KEY,
                    zwave_controller_get_key_from_encapsulation(
                      ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0));

  TEST_ASSERT_EQUAL(0x00,
                    zwave_controller_get_key_from_encapsulation(
                      ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME));

  TEST_ASSERT_EQUAL(0x00,
                    zwave_controller_get_key_from_encapsulation(
                      ZWAVE_CONTROLLER_ENCAPSULATION_NONE));
}

void test_zwave_controller_get_highest_encapsulation()
{
  TEST_ASSERT_EQUAL(
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS,
    zwave_controller_get_highest_encapsulation(ZWAVE_CONTROLLER_S2_ACCESS_KEY));

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS,
                    zwave_controller_get_highest_encapsulation(0xFF));

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED,
                    zwave_controller_get_highest_encapsulation(
                      ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY));

  TEST_ASSERT_EQUAL(
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0,
    zwave_controller_get_highest_encapsulation(ZWAVE_CONTROLLER_S0_KEY));

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    zwave_controller_get_highest_encapsulation(0x00));
}