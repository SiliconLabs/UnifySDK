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
// Includes from this component
#include "zpc_attribute_mapper.h"

// Test includes
#include "unity.h"

// Mock includes
#include "attribute_mapper_mock.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_zpc_attribute_mapper()
{
  // Not much to test, just run the init
  sl_status_t test_status = SL_STATUS_NONE_WAITING;

  attribute_mapper_set_endpoint_id_attribute_type_Expect(4);
  attribute_mapper_init_ExpectAndReturn(test_status);
  TEST_ASSERT_EQUAL(test_status, zpc_attribute_mapper_init());
}