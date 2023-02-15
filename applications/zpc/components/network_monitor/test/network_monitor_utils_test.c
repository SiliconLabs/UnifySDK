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
#include "network_monitor_utils.h"
#include "unity.h"

// Unify components
#include "datastore.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"

// ZPC Components
#include "attribute_store_defined_attribute_types.h"
#include "zpc_attribute_store_type_registration.h"

// Test helpers
#include "unity.h"
#include "zpc_attribute_store_test_helper.h"
#include "contiki_test_helper.h"

// Mocks
#include "attribute_resolver_mock.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zpc_attribute_store_register_known_attribute_types();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  contiki_test_helper_init();
}

void test_network_monitor_utils_is_node_asleep()
{
  is_node_or_parent_paused_ExpectAndReturn(23, true);
  TEST_ASSERT_TRUE(network_monitor_is_node_asleep_due_to_inactivity(23, 1));

  clock_time_t current_time = clock_seconds();
  clock_time_t last_rx_tx   = current_time;
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_LAST_RECEIVED_FRAME_TIMESTAMP,
                                     &current_time,
                                     sizeof(current_time));

  is_node_or_parent_paused_IgnoreAndReturn(false);
  contiki_test_helper_run(1000);
  current_time = clock_seconds();
  TEST_ASSERT_FALSE(network_monitor_is_node_asleep_due_to_inactivity(
    node_id_node,
    (current_time - last_rx_tx) + 1));

  TEST_ASSERT_TRUE(network_monitor_is_node_asleep_due_to_inactivity(
    node_id_node,
    (current_time - last_rx_tx) - 1));
}

void test_network_monitor_get_operating_mode()
{
  // If the NIF falgs are not present, zwave_get_operating_mode will return unknown.
  // Here we just test the wrapper.
  TEST_ASSERT_EQUAL(OPERATING_MODE_UNKNOWN,
                    network_monitor_get_operating_mode(0));
  TEST_ASSERT_EQUAL(OPERATING_MODE_UNKNOWN,
                    network_monitor_get_operating_mode(node_id_node));
}