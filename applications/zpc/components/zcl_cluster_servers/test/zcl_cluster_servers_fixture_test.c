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
#include "unity.h"
#include "zcl_cluster_servers.h"

#include "datastore.h"
#include "attribute_store_fixt.h"
#include "zpc_config_mock.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_zcl_cluster_servers_fixture_test()
{
  // Mock the configuration
  zpc_config_t test_configuration   = {0};
  test_configuration.ota_cache_path = ".";
  test_configuration.ota_cache_size = 123;
  zpc_get_config_IgnoreAndReturn(&test_configuration);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, zcl_cluster_servers_init());
  TEST_ASSERT_EQUAL(0, zcl_cluster_servers_teardown());
}