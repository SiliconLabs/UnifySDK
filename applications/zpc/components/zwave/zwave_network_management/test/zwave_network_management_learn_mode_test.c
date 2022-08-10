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

static void refresh_cached_id_changed_home_id_stub(int cmock_num_calls)
{
  // Simulate an update of our NodeID and HomeID.
  nms.cached_local_node_id += 1;
  nms.cached_home_id += 1;
}

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
  TEST_ASSERT_EQUAL(0, zwave_network_management_fixt_teardown());
}

void test_zwave_network_management_activate_learn_mode_nwi_happy_case()
{
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  // Activate NWI learn mode:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_learn_mode(ZWAVE_NETWORK_MANAGEMENT_LEARN_NWI));

  network_management_stop_ongoing_operations_Expect();
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_NWI,
                                       &on_learn_mode_callback,
                                       SL_STATUS_OK);
  contiki_test_helper_run(1);
  // State must have been updated
  TEST_ASSERT_EQUAL(NM_LEARN_MODE, zwave_network_management_get_state());
  TEST_ASSERT_EQUAL(ZWAVE_NETWORK_MANAGEMENT_LEARN_NWI, nms.learn_mode_intent);
}

void test_zwave_network_management_stop_learn_mode_happy_case()
{
  // Put ourself in Learn mode
  nms.state = NM_LEARN_MODE;
  TEST_ASSERT_EQUAL(NM_LEARN_MODE, zwave_network_management_get_state());

  // Activate NWI learn mode:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_learn_mode(ZWAVE_NETWORK_MANAGEMENT_LEARN_NONE));

  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);
  reset_nms_last_operation_data_Expect();

  contiki_test_helper_run(1);
  // State must have been updated to IDLE
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_stop_learn_mode_while_idle()
{
  // Start from IDLE
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  // Try to stop Learn mode
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_learn_mode(ZWAVE_NETWORK_MANAGEMENT_LEARN_NONE));

  // Nothing should happen, we were IDLE.

  contiki_test_helper_run(1);
  // State must have been updated to IDLE
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
  TEST_ASSERT_EQUAL(ZWAVE_NETWORK_MANAGEMENT_LEARN_NONE, nms.learn_mode_intent);
}

void test_zwave_network_management_stop_learn_mode_while_add_mode()
{
  // Put ourself in Add mode
  nms.state = NM_WAITING_FOR_ADD;
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());

  // Try to stop Learn mode
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_learn_mode(ZWAVE_NETWORK_MANAGEMENT_LEARN_NONE));

  // Nothing should happen, we were in Add node..

  contiki_test_helper_run(1);
  // State must have been updated to IDLE
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());
  TEST_ASSERT_EQUAL(ZWAVE_NETWORK_MANAGEMENT_LEARN_NONE, nms.learn_mode_intent);
}

void test_zwave_network_management_start_learn_mode_while_add_mode()
{
  // Put ourself in Add mode
  nms.state = NM_WAITING_FOR_ADD;
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());

  // Try to stop Learn mode
  TEST_ASSERT_EQUAL(
    SL_STATUS_BUSY,
    zwave_network_management_learn_mode(ZWAVE_NETWORK_MANAGEMENT_LEARN_NWI));

  // Nothing should happen, we were in Add node..

  contiki_test_helper_run(1);
  // State must have been updated to IDLE
  TEST_ASSERT_EQUAL(NM_WAITING_FOR_ADD, zwave_network_management_get_state());
  TEST_ASSERT_EQUAL(ZWAVE_NETWORK_MANAGEMENT_LEARN_NONE, nms.learn_mode_intent);
}

void test_zwave_network_management_learn_mode_nwi_timeout()
{
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  // Activate NWI learn mode:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_learn_mode(ZWAVE_NETWORK_MANAGEMENT_LEARN_NWI));

  network_management_stop_ongoing_operations_Expect();
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_NWI,
                                       &on_learn_mode_callback,
                                       SL_STATUS_OK);
  contiki_test_helper_run(1);

  // State must have been updated
  TEST_ASSERT_EQUAL(NM_LEARN_MODE, zwave_network_management_get_state());
  TEST_ASSERT_EQUAL(ZWAVE_NETWORK_MANAGEMENT_LEARN_NWI, nms.learn_mode_intent);

  // Pass the timeout, it will tell the Z-Wave API to abort
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);
  reset_nms_last_operation_data_Expect();
  contiki_test_helper_run(LEARN_MODE_TIMEOUT * 1000 + 1);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_learn_mode_nwe_timeout()
{
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  // Activate NWI learn mode:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_learn_mode(ZWAVE_NETWORK_MANAGEMENT_LEARN_NWE));

  network_management_stop_ongoing_operations_Expect();
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_NWE,
                                       &on_learn_mode_callback,
                                       SL_STATUS_OK);
  contiki_test_helper_run(1);

  // State must have been updated
  TEST_ASSERT_EQUAL(NM_LEARN_MODE, zwave_network_management_get_state());
  TEST_ASSERT_EQUAL(ZWAVE_NETWORK_MANAGEMENT_LEARN_NWE, nms.learn_mode_intent);

  // Pass the timeout, it will tell the Z-Wave API to abort
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);
  reset_nms_last_operation_data_Expect();
  contiki_test_helper_run(LEARN_MODE_TIMEOUT * 1000 + 1);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_learn_mode_nwe_successful_exclusion()
{
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  // Activate NWI learn mode:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_learn_mode(ZWAVE_NETWORK_MANAGEMENT_LEARN_NWE));

  network_management_stop_ongoing_operations_Expect();
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_NWE,
                                       &on_learn_mode_callback,
                                       SL_STATUS_OK);
  contiki_test_helper_run(1);

  // State must have been updated
  TEST_ASSERT_EQUAL(NM_LEARN_MODE, zwave_network_management_get_state());
  TEST_ASSERT_EQUAL(ZWAVE_NETWORK_MANAGEMENT_LEARN_NWE, nms.learn_mode_intent);

  // Callback:
  LEARN_INFO test_learn_mode_info = {};
  test_learn_mode_info.bStatus    = LEARN_MODE_STARTED;
  on_learn_mode_callback(&test_learn_mode_info);

  // Run only 1 event
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(NM_LEARN_MODE_STARTED,
                    zwave_network_management_get_state());

  // Second callback:
  test_learn_mode_info.bStatus = LEARN_MODE_DONE;
  on_learn_mode_callback(&test_learn_mode_info);

  //it should "set default" on exclusion success
  network_management_refresh_cached_ids_Expect();
  network_management_refresh_cached_node_list_Expect();

  contiki_test_helper_run_once(1);
  TEST_ASSERT_EQUAL(NM_SET_DEFAULT, zwave_network_management_get_state());

  // Then it completes the set default and goes back to idle.
  network_management_refresh_network_information_Expect();
  zwave_s2_create_new_network_keys_Expect();
  zwave_s2_network_init_Expect();
  // on_new_network()
  network_management_refresh_network_information_Expect();
  zwave_s2_keystore_get_assigned_keys_ExpectAndReturn(2);
  network_management_take_sis_role_if_no_suc_in_network_Expect();
  reset_nms_last_operation_data_Expect();
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_learn_mode_nwi_successful_inclusion()
{
  // Start straight up in LEARN MODE
  nms.state                 = NM_LEARN_MODE;
  nms.learn_mode_intent     = ZWAVE_NETWORK_MANAGEMENT_LEARN_NWI;
  nms.node_id_being_handled = 214;
  TEST_ASSERT_EQUAL(NM_LEARN_MODE, zwave_network_management_get_state());

  // Callback that something happened:
  LEARN_INFO test_learn_mode_info = {};
  test_learn_mode_info.bStatus    = LEARN_MODE_STARTED;
  on_learn_mode_callback(&test_learn_mode_info);

  // Run events. we expect to open up for S0 and S2 Security Bootstrapping here.
  zwave_s0_start_learn_mode_Expect(nms.node_id_being_handled);
  zwave_s2_start_learn_mode_Expect(nms.node_id_being_handled);
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(NM_LEARN_MODE_STARTED,
                    zwave_network_management_get_state());

  // Second callback:
  test_learn_mode_info.bStatus = LEARN_MODE_DONE;
  on_learn_mode_callback(&test_learn_mode_info);

  // Now we should be ready for boostrapping, if our HomeID / NodeID were
  // just updated
  network_management_refresh_cached_ids_Stub(
    &refresh_cached_id_changed_home_id_stub);
  network_management_refresh_cached_node_list_Expect();
  zwave_s2_keystore_reset_assigned_keys_Expect();
  zwave_s2_neighbor_discovery_complete_Expect();

  contiki_test_helper_run_once(1);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_LEARN,
                    zwave_network_management_get_state());
}

void test_zwave_network_management_learn_mode_failed_event()
{
  // Start straight up in LEARN MODE Started
  nms.state = NM_LEARN_MODE_STARTED;
  TEST_ASSERT_EQUAL(NM_LEARN_MODE_STARTED,
                    zwave_network_management_get_state());

  // Callback that something failed
  LEARN_INFO test_learn_mode_info = {};
  test_learn_mode_info.bStatus    = LEARN_MODE_FAILED;
  on_learn_mode_callback(&test_learn_mode_info);

  // Run events. We expect to stop learn mode and go back to idle
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);
  reset_nms_last_operation_data_Expect();
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_controller_replication_successful()
{
  // Start straight up in LEARN MODE
  nms.state = NM_LEARN_MODE;
  TEST_ASSERT_EQUAL(NM_LEARN_MODE, zwave_network_management_get_state());

  // Callback with Learn Mode Done:
  LEARN_INFO test_learn_mode_info = {};
  test_learn_mode_info.bStatus    = LEARN_MODE_DONE;
  on_learn_mode_callback(&test_learn_mode_info);

  reset_nms_last_operation_data_Expect();
  contiki_test_helper_run(1);
  // State must have been updated to IDLE
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_on_learn_mode_callback_with_unknown_value()
{
  // Start straight up in LEARN MODE
  nms.state = NM_LEARN_MODE;
  TEST_ASSERT_EQUAL(NM_LEARN_MODE, zwave_network_management_get_state());

  // Get a callback with unknown learn mode status.
  LEARN_INFO test_learn_mode_info = {};
  test_learn_mode_info.bStatus    = 0xFF;
  on_learn_mode_callback(&test_learn_mode_info);

  // Nothing should happen, still in learn mode.
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(NM_LEARN_MODE, zwave_network_management_get_state());
}

void test_zwave_network_management_on_learn_mode_callback_with_value_5()
{
  // Start straight up in LEARN MODE
  nms.state = NM_LEARN_MODE;
  TEST_ASSERT_EQUAL(NM_LEARN_MODE, zwave_network_management_get_state());

  // Get a callback with learn mode status 5, which is illegal but was returned
  // by the Z-Wave API at some point instead of 6..
  LEARN_INFO test_learn_mode_info = {};
  test_learn_mode_info.bStatus    = 5;
  on_learn_mode_callback(&test_learn_mode_info);

  reset_nms_last_operation_data_Expect();
  contiki_test_helper_run(1);
  // State must have been updated to IDLE
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}
