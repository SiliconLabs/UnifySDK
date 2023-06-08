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
#include "zwave_rx_callbacks_mock.h"

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
  zwave_rx_register_zwave_api_started_callback_ExpectAndReturn(
    &on_zwave_api_started,
    SL_STATUS_OK);
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

void test_zwave_network_management_request_node_neighbor_discovery()
{
  zwave_node_id_t test_node_id = 1;
  nms.state                    = NM_IDLE;
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  network_management_is_ready_for_a_new_operation_ExpectAndReturn(true);
  zwave_network_management_request_node_neighbor_discovery(test_node_id);

  zwapi_request_neighbor_update_ExpectAndReturn(test_node_id,
                                                NULL,
                                                SL_STATUS_OK);
  zwapi_request_neighbor_update_IgnoreArg_completedFunc();
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_NEIGHBOR_DISCOVERY_ONGOING,
                    zwave_network_management_get_state());

  // Post a fake event, nothing should happen
  process_post(&zwave_network_management_process, NM_EV_REMOVE_FAILED, NULL);

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_NEIGHBOR_DISCOVERY_ONGOING,
                    zwave_network_management_get_state());

  uint8_t neighbor_update_status = 0x21;  // Discovery started
  on_request_node_neighbor_update_completed(neighbor_update_status);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_NEIGHBOR_DISCOVERY_ONGOING,
                    zwave_network_management_get_state());

  // Discovery completed
  neighbor_update_status = 0x22;
  on_request_node_neighbor_update_completed(neighbor_update_status);
  reset_nms_last_operation_data_Expect();
  contiki_test_helper_run(10);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_abort_request_node_neighbor_discovery()
{
  zwave_node_id_t test_node_id = 0x55;
  nms.state                    = NM_IDLE;
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  network_management_is_ready_for_a_new_operation_ExpectAndReturn(true);
  zwave_network_management_request_node_neighbor_discovery(test_node_id);

  zwapi_request_neighbor_update_ExpectAndReturn(test_node_id,
                                                NULL,
                                                SL_STATUS_OK);
  zwapi_request_neighbor_update_IgnoreArg_completedFunc();
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_NEIGHBOR_DISCOVERY_ONGOING,
                    zwave_network_management_get_state());

  zwave_network_management_abort();
  reset_nms_last_operation_data_Expect();
  contiki_test_helper_run(10);

  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_node_neighbor_discovery_rejected()
{
  zwave_node_id_t test_node_id = 300;
  nms.state                    = NM_IDLE;
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  network_management_is_ready_for_a_new_operation_ExpectAndReturn(true);
  zwave_network_management_request_node_neighbor_discovery(test_node_id);
  zwapi_request_neighbor_update_ExpectAndReturn(test_node_id,
                                                NULL,
                                                SL_STATUS_FAIL);
  zwapi_request_neighbor_update_IgnoreArg_completedFunc();
  reset_nms_last_operation_data_Expect();
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}