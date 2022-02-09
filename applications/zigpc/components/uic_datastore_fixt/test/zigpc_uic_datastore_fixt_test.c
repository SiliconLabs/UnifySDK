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
// Includes from this component
#include "zigpc_uic_datastore_fixt.h"

// Test includes
#include "unity.h"

// Unify component includes
#include "datastore_fixt_mock.h"
#include "zigpc_config_mock.h"

// Generic includes
#include <string.h>

// Static test variables
zigpc_config_t zigpc_config;

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
  memset(&zigpc_config, 0, sizeof(zigpc_config));
}

void test_zigpc_datastore_test()
{
  zigpc_config.datastore_file = "Test_file.db";
  // Not much to test, just run the init
  sl_status_t test_status = SL_STATUS_NONE_WAITING;

  zigpc_get_config_ExpectAndReturn(&zigpc_config);
  datastore_fixt_setup_and_handle_version_ExpectAndReturn(
    zigpc_config.datastore_file,
    ZIGPC_CURRENT_DATASTORE_VERSION,
    test_status);

  TEST_ASSERT_EQUAL(test_status, zigpc_uic_datastore_fixt_setup());
}