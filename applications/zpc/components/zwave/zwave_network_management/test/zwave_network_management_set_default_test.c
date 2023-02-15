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
#include "nm_state_machine.h"
#include "zwave_network_management.h"
#include "zwave_network_management_fixt.h"
#include "zwave_network_management_process.h"
#include "zwave_network_management_callbacks.h"

// Test helpers and test includes
#include "unity.h"
#include "contiki_test_helper.h"

// Mock includes
#include "zwave_network_management_helpers_mock.h"
#include "zwave_s2_keystore_mock.h"
#include "zwave_s2_network_mock.h"
#include "zwave_s0_network_mock.h"
#include "zwapi_protocol_controller_mock.h"
#include "zwapi_protocol_basis_mock.h"
#include "zwave_rx_mock.h"

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
  network_management_refresh_network_information_Expect();
  network_management_stop_ongoing_operations_Expect();
  network_management_take_sis_role_if_no_suc_in_network_Expect();
  zwave_s2_set_network_callbacks_Ignore();
  zwave_s0_set_network_callbacks_Ignore();
  zwave_s2_keystore_get_dsk_Ignore();
  zwave_network_management_fixt_setup();

  zwave_s2_set_network_callbacks_StopIgnore();
  zwave_s0_set_network_callbacks_StopIgnore();

  contiki_test_helper_init();
  process_start(&zwave_network_management_process, NULL);
}

/// Called after each and every test
void tearDown()
{
  network_management_stop_ongoing_operations_Expect();
  process_exit(&zwave_network_management_process);
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(0, zwave_network_management_fixt_teardown());
}

void test_zwave_network_management_set_default_happy_case()
{
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  // Ask to set default while idle
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_network_management_set_default());

  zwapi_set_default_ExpectAndReturn(&on_set_default_complete, SL_STATUS_OK);
  contiki_test_helper_run(1);
  // State must have been updated
  TEST_ASSERT_EQUAL(NM_SET_DEFAULT, zwave_network_management_get_state());

  on_set_default_complete();
  network_management_refresh_network_information_Expect();
  zwave_s2_create_new_network_keys_Expect();
  zwave_s2_network_init_Expect();
  network_management_refresh_network_information_Expect();
  zwave_s2_keystore_get_assigned_keys_ExpectAndReturn(34);
  network_management_take_sis_role_if_no_suc_in_network_Expect();
  reset_nms_last_operation_data_Expect();
  contiki_test_helper_run(1);
  // State must have been updated
  TEST_ASSERT_EQUAL(34, nms.granted_keys);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_set_default_while_assigning_return_routes()
{
  // Pretend we are assigning return routes
  nms.state = NM_ASSIGNING_RETURN_ROUTE;
  TEST_ASSERT_EQUAL(NM_ASSIGNING_RETURN_ROUTE,
                    zwave_network_management_get_state());

  // Ask to set default, it should work
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_network_management_set_default());
}

void test_zwave_network_management_set_default_while_busy()
{
  // Pretend we are busy
  nms.state = NM_SEND_NOP;
  TEST_ASSERT_EQUAL(NM_SEND_NOP, zwave_network_management_get_state());

  // Ask to set default, it should work
  TEST_ASSERT_EQUAL(SL_STATUS_BUSY, zwave_network_management_set_default());
}

void test_zwave_network_management_set_no_zwave_api_callback()
{
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  // Ask to set default while idle
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_network_management_set_default());

  zwapi_set_default_ExpectAndReturn(&on_set_default_complete, SL_STATUS_OK);
  contiki_test_helper_run(1);
  // State must have been updated
  TEST_ASSERT_EQUAL(NM_SET_DEFAULT, zwave_network_management_get_state());

  contiki_test_helper_run(1000);
  TEST_ASSERT_EQUAL(NM_SET_DEFAULT, zwave_network_management_get_state());

  // Now we wait very long and the Z-Wave API never calls back.
  // In this case, we will call soft-reset
  zwapi_soft_reset_ExpectAndReturn(SL_STATUS_OK);
  zwave_rx_wait_for_zwave_api_to_be_ready_Expect();

  network_management_refresh_network_information_Expect();
  zwave_s2_create_new_network_keys_Expect();
  zwave_s2_network_init_Expect();
  network_management_refresh_network_information_Expect();
  zwave_s2_keystore_get_assigned_keys_ExpectAndReturn(88);
  network_management_take_sis_role_if_no_suc_in_network_Expect();
  reset_nms_last_operation_data_Expect();

  contiki_test_helper_run(SET_DEFAULT_TIMEOUT);

  // State must have been updated
  TEST_ASSERT_EQUAL(88, nms.granted_keys);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}
