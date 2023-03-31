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

void test_zwave_network_management_activate_smartstart_add_mode_controller_reset_ongoing()
{
  // Make sure to be the SIS, so we can do SmartStart Add Mode:
  nms.controller_capabilities_bitmask
    = CONTROLLER_NODEID_SERVER_PRESENT | CONTROLLER_IS_SUC;
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  // SmartStart add mode enabled if we are IDLE:
  zwave_network_management_enable_smart_start_add_mode(true);

  zwapi_add_node_to_network_ExpectAndReturn(ADD_NODE_SMART_START
                                              | ADD_NODE_OPTION_NETWORK_WIDE,
                                            NULL,
                                            SL_STATUS_OK);
  zwapi_set_learn_mode_ExpectAndReturn(LEARN_MODE_DISABLE, NULL, SL_STATUS_OK);
  reset_nms_last_operation_data_Expect();

  contiki_test_helper_run(1);
  // We stay in IDLE while in SmartStart Add mode
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());

  // Now we have an incoming candidate, at that point we can accept it:
  network_management_is_ready_for_a_new_operation_ExpectAndReturn(true);
  const zwave_dsk_t dsk                = {};
  zwave_keyset_t granted_keys          = 0x34;
  zwave_protocol_t preferred_inclusion = PROTOCOL_ZWAVE_LONG_RANGE;
  zwave_network_management_add_node_with_dsk(dsk,
                                             granted_keys,
                                             preferred_inclusion);

  // Run the events, but in the meantime we have changed our mind and e.g.
  // the Z-Wave controller is resetting.
  // We should go back to idle and wait.
  network_management_is_ready_for_a_new_operation_ExpectAndReturn(false);
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}
