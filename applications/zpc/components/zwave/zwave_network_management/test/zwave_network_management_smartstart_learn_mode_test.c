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

void test_zwave_network_management_activate_smartstart_learn_mode_happy_case()
{
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  // SmartStart learn mode enabled if we are IDLE:
  zwave_network_management_enable_smart_start_learn_mode(true);

  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, SL_STATUS_OK);
  reset_nms_last_operation_data_Expect();
  we_are_alone_in_our_network_ExpectAndReturn(true);
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_SMART_START,
                                       &on_learn_mode_callback,
                                       SL_STATUS_OK);
  contiki_test_helper_run(1);
  // We stay in IDLE while in SmartStart Learn mode
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_activate_smartstart_learn_mode_not_alone_in_network()
{
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  // if we are not alone in our network, it will not be called:
  zwave_network_management_enable_smart_start_learn_mode(true);

  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, SL_STATUS_OK);
  reset_nms_last_operation_data_Expect();
  we_are_alone_in_our_network_ExpectAndReturn(false);

  contiki_test_helper_run(1);
  // We stay in IDLE while in SmartStart Learn mode
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_activate_smartstart_learn_mode_add_mode_is_aready_enabled()
{
  // Make sure we are the SIS, so we activate SmartStart Add mode.
  nms.controller_capabilities_bitmask
    = CONTROLLER_NODEID_SERVER_PRESENT | CONTROLLER_IS_SUC;
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  // Now if SmartStart Add mode is enabled, we do not start SmartStart Learn mode
  zwave_network_management_enable_smart_start_learn_mode(true);
  zwave_network_management_enable_smart_start_add_mode(true);
  // Expectations are duplicated because events are posted twice
  reset_nms_last_operation_data_Expect();
  reset_nms_last_operation_data_Expect();
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_SMART_START
                                              | ADD_NODE_OPTION_NETWORK_WIDE,
                                            NULL,
                                            SL_STATUS_OK);
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_SMART_START
                                              | ADD_NODE_OPTION_NETWORK_WIDE,
                                            NULL,
                                            SL_STATUS_OK);

  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_activate_smartstart_learn_mode_disabled()
{
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  // Now if SmartStart Add mode is enabled, we do not start SmartStart Learn mode
  zwave_network_management_enable_smart_start_learn_mode(false);

  // Expectations are duplicated because events are posted twice
  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_STOP, NULL, SL_STATUS_OK);
  reset_nms_last_operation_data_Expect();

  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_network_management_smartstart_learn_mode_callback_happy_case()
{
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
  zwave_node_id_t test_node_id = 23;
  LEARN_INFO learn_mode_info   = {};
  learn_mode_info.bStatus      = LEARN_MODE_STARTED;
  learn_mode_info.bSource      = test_node_id;

  // Now simulate we were in SmartStart learn mode just by changing our learn mode intent
  nms.learn_mode_intent = ZWAVE_NETWORK_MANAGEMENT_LEARN_SMART_START;

  on_learn_mode_callback(&learn_mode_info);
  TEST_ASSERT_EQUAL(NM_LEARN_MODE, zwave_network_management_get_state());

  // When running the events, we will open for S2 bootstrapping
  zwave_s2_start_learn_mode_Expect(test_node_id);
  contiki_test_helper_run(1);
  // The Network management state will have changed to Learn Mode Started
  TEST_ASSERT_EQUAL(NM_LEARN_MODE_STARTED,
                    zwave_network_management_get_state());
}

void test_zwave_network_management_smartstart_learn_mode_complete_s2_boostrapping_starts()
{
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  // Now simulate we were in SmartStart learn mode just by changing our learn mode intent
  nms.learn_mode_intent = ZWAVE_NETWORK_MANAGEMENT_LEARN_SMART_START;
  nms.state             = NM_LEARN_MODE_STARTED;

  process_post(&zwave_network_management_process, NM_EV_LEARN_DONE, NULL);

  // Now we wait for the S2 boostrapping callback
  network_management_refresh_cached_ids_Stub(
    &refresh_cached_id_changed_home_id_stub);
  network_management_refresh_cached_node_list_Expect();
  zwave_s2_keystore_reset_assigned_keys_Expect();
  zwave_s2_neighbor_discovery_complete_Expect();
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_LEARN,
                    zwave_network_management_get_state());
}

void test_zwave_network_management_smartstart_learn_mode_self_destruct()
{
  // Now simulate we were in SmartStart learn mode, waiting for a Security bootstrapping
  nms.learn_mode_intent = ZWAVE_NETWORK_MANAGEMENT_LEARN_SMART_START;
  nms.state             = NM_WAIT_FOR_SECURE_LEARN;
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_LEARN,
                    zwave_network_management_get_state());

  // Simulate the outcome of Security bootstrapping:
  process_post(&zwave_network_management_process, NM_EV_SECURITY_DONE, NULL);

  // Running contiki move us to Set Default.
  we_should_self_destruct_ExpectAndReturn(true);
  zwapi_set_default_ExpectAndReturn(&on_set_default_complete, SL_STATUS_OK);
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(NM_SET_DEFAULT, zwave_network_management_get_state());

  // Z-Wave API calls back when Set default is completed:
  on_set_default_complete();
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

void test_zwave_network_management_smartstart_learn_mode_bootstrapping_complete()
{
  // Now simulate we were in SmartStart learn mode, waiting for a Security bootstrapping
  nms.learn_mode_intent = ZWAVE_NETWORK_MANAGEMENT_LEARN_SMART_START;
  nms.state             = NM_WAIT_FOR_SECURE_LEARN;
  TEST_ASSERT_EQUAL(NM_WAIT_FOR_SECURE_LEARN,
                    zwave_network_management_get_state());

  // Simulate the outcome of Security bootstrapping:
  process_post(&zwave_network_management_process, NM_EV_SECURITY_DONE, NULL);

  // Running contiki move us to entering our new network.
  we_should_self_destruct_ExpectAndReturn(false);

  // on_new_network()
  network_management_refresh_network_information_Expect();
  zwave_s2_keystore_get_assigned_keys_ExpectAndReturn(2);
  network_management_take_sis_role_if_no_suc_in_network_Expect();
  reset_nms_last_operation_data_Expect();

  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}