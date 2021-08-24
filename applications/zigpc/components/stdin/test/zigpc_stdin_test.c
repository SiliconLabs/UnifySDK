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

// Test framework
#include "unity.h"

// UIC Component
#include "sl_status.h"
#include "uic_stdin_test.h"

// ZigPC Components mocks
#include "zigpc_gateway_mock.h"

// Own component
#include "zigpc_stdin_fixt.h"

/**
 * @brief Setup the test suite (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp(void)
{
  zigpc_stdin_setup_fixt();
}

/**
 * @brief Teardown the test suite (called once after all test_xxx functions are called)
 *
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/**
 * @brief Setup before each test case
 *
 */
void setUp(void) {}

void test_zwave_set_default()
{
  // ARRANGE
  sl_status_t status;
  zigpc_gateway_command_print_info_Expect();

  // ACT
  status = uic_stdin_handle_command("info");

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}
