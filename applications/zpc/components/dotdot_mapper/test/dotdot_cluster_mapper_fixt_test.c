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
#include "dotdot_mapper_fixt.h"
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

/// Called after each and every test
void tearDown() {}

void test_level_cluster_mapper_test()
{
  // Nothing to test really. Call init and teardown.
  TEST_ASSERT_EQUAL(SL_STATUS_OK, dotdot_mapper_init());

  // Then teardown
  TEST_ASSERT_EQUAL(0, dotdot_mapper_teardown());
}