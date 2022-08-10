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
#include "zwave_network_management_callbacks.h"

// Test includes
#include "unity.h"

// Mocks
#include "zwave_network_management_helpers_mock.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_on_set_suc_node_id_completed()
{
  // Set SUC callback triggers a refresh of the controller's capabilities, not matter the status.
  uint8_t set_suc_status = 0;

  network_management_refresh_controller_capabilities_bitmask_Expect();
  on_set_suc_node_id_completed(set_suc_status);

  set_suc_status = 5;
  network_management_refresh_controller_capabilities_bitmask_Expect();
  on_set_suc_node_id_completed(set_suc_status);

  set_suc_status = 6;
  network_management_refresh_controller_capabilities_bitmask_Expect();
  on_set_suc_node_id_completed(set_suc_status);

  set_suc_status = 255;
  network_management_refresh_controller_capabilities_bitmask_Expect();
  on_set_suc_node_id_completed(set_suc_status);
}