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
#include "aoxpc_datastore_fixt.h"

// Test includes
#include "unity.h"

// AoXPC Components
#include "aoxpc_config.h"

// Mock includes
#include "datastore_fixt_mock.h"
#include "config_mock.h"

// Generic includes
#include <string.h>

// Test constants
const char *test_database_file      = "/var/shared/uic/aoxpc_database.db";

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_aoxpc_datastore_test()
{
  config_get_as_string_ExpectAndReturn(CONFIG_KEY_AOXPC_DATASTORE_FILE,
                                       0,
                                       CONFIG_STATUS_OK);
  config_get_as_string_IgnoreArg_result();
  config_get_as_string_ReturnThruPtr_result(&test_database_file);

  // Not much to test, just run the init
  sl_status_t test_status = SL_STATUS_NONE_WAITING;

  datastore_fixt_setup_and_handle_version_ExpectAndReturn(
    test_database_file,
    AOXPC_DATASTORE_VERSION,
    test_status);

  TEST_ASSERT_EQUAL(test_status, aoxpc_datastore_fixt_setup());
}