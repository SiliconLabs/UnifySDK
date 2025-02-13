/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "eed_datastore_fixt.h"

// Test includes
#include "unity.h"

// Unify component includes
#include "datastore_fixt_mock.h"
#include "eed_config_mock.h"

// Generic includes
#include <string.h>

// Static test variables
eed_config_t eed_config;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  memset(&eed_config, 0, sizeof(eed_config));
}

void test_eed_datastore_test()
{
  eed_config.datastore_file = "Test_file.db";
  // Not much to test, just run the init
  sl_status_t test_status = SL_STATUS_NONE_WAITING;

  eed_get_config_ExpectAndReturn(&eed_config);
  datastore_fixt_setup_and_handle_version_ExpectAndReturn(
    eed_config.datastore_file,
    EED_DATASTORE_VERSION,
    test_status);

  TEST_ASSERT_EQUAL(test_status, eed_datastore_fixt_setup());
}