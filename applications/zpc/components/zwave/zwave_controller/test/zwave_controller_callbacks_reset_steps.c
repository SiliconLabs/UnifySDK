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
#include "zwave_controller_callbacks.h"
#include "zwave_controller.h"

#include <stdbool.h>

#include "unity.h"
#include "cmock.h"
#include "sl_log.h"

/// Static variables
static uint8_t reset_step_1_count = 0;
static uint8_t reset_step_2_count = 0;
static uint8_t reset_step_3_count = 0;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp()
{
  reset_step_1_count = 0;
  reset_step_2_count = 0;
  reset_step_3_count = 0;
  zwave_controller_callbacks_init();
}

static sl_status_t reset_step_1()
{
  reset_step_1_count += 1;
  return SL_STATUS_OK;
}

static sl_status_t reset_step_2()
{
  reset_step_2_count += 1;
  return SL_STATUS_FAIL;
}

static sl_status_t reset_step_3()
{
  reset_step_3_count += 1;
  return SL_STATUS_OK;
}

/// Test of callback registration
void test_zwave_controller_reset_steps_registration()
{
  // try to register a NULL pointer callback
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_controller_register_reset_step(NULL, 1));

  // Wrong priority
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_controller_register_reset_step(reset_step_1, -1));

  // Registration
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_controller_register_reset_step(&reset_step_1, 1));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_controller_register_reset_step(&reset_step_2, 2));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_controller_register_reset_step(&reset_step_2, 2));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_controller_register_reset_step(&reset_step_3, 3));

  // Invoke Z-Wave Controller reset.
  zwave_controller_on_reset_step_complete(2);
  TEST_ASSERT_EQUAL(0, reset_step_1_count);
  TEST_ASSERT_EQUAL(0, reset_step_2_count);
  TEST_ASSERT_EQUAL(0, reset_step_3_count);

  TEST_ASSERT_FALSE(zwave_controller_is_reset_ongoing());

  zwave_controller_reset();

  TEST_ASSERT_TRUE(zwave_controller_is_reset_ongoing());

  // First step says OK, we wait for a callback:
  TEST_ASSERT_EQUAL(1, reset_step_1_count);
  TEST_ASSERT_EQUAL(0, reset_step_2_count);
  TEST_ASSERT_EQUAL(0, reset_step_3_count);

  zwave_controller_on_reset_step_complete(1);

  TEST_ASSERT_TRUE(zwave_controller_is_reset_ongoing());

  // Step 2 says FAIL; we continue with step 3.
  TEST_ASSERT_EQUAL(1, reset_step_1_count);
  TEST_ASSERT_EQUAL(1, reset_step_2_count);
  TEST_ASSERT_EQUAL(1, reset_step_3_count);

  TEST_ASSERT_TRUE(zwave_controller_is_reset_ongoing());
  zwave_controller_on_reset_step_complete(3);
  TEST_ASSERT_FALSE(zwave_controller_is_reset_ongoing());
}
