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

// Generic includes
#include <string.h>

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

void test_zwave_network_management_is_protocol_sending_frames_to_node()
{
  nms.state                    = NM_IDLE;
  zwave_node_id_t test_node_id = 23;
  nms.node_id_being_handled    = test_node_id;

  TEST_ASSERT_FALSE(
    zwave_network_management_is_protocol_sending_frames_to_node(test_node_id));

  nms.state = NM_ASSIGNING_RETURN_ROUTE;
  TEST_ASSERT_FALSE(
    zwave_network_management_is_protocol_sending_frames_to_node(test_node_id));

  nms.state = NM_NEIGHBOR_DISCOVERY_ONGOING;
  TEST_ASSERT_TRUE(
    zwave_network_management_is_protocol_sending_frames_to_node(test_node_id));

  TEST_ASSERT_FALSE(zwave_network_management_is_protocol_sending_frames_to_node(
    test_node_id + 1));

  nms.state = NM_IDLE;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_assign_return_route(2, test_node_id));

  TEST_ASSERT_FALSE(
    zwave_network_management_is_protocol_sending_frames_to_node(test_node_id));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_assign_return_route(test_node_id, 1));

  TEST_ASSERT_TRUE(
    zwave_network_management_is_protocol_sending_frames_to_node(test_node_id));
}

void test_zwave_network_management_is_busy()
{
  nms.state = NM_IDLE;
  TEST_ASSERT_FALSE(zwave_network_management_is_busy());

  for (int i = NM_IDLE + 1; i < 50; i++) {
    nms.state = i;
    TEST_ASSERT_TRUE(zwave_network_management_is_busy());
  }
}

void test_zwave_network_management_get_network_size()
{
  nms.state = NM_IDLE;
  memset(nms.cached_node_list, 0, sizeof(zwave_nodemask_t));

  TEST_ASSERT_EQUAL(0, zwave_network_management_get_network_size());

  for (zwave_node_id_t n = ZW_MIN_NODE_ID; n <= ZW_MAX_NODES; n++) {
    ZW_ADD_NODE_TO_MASK(n, nms.cached_node_list);
    TEST_ASSERT_EQUAL(n, zwave_network_management_get_network_size());
  }

  for (zwave_node_id_t n = ZW_LR_MIN_NODE_ID; n <= ZW_LR_MAX_NODE_ID; n++) {
    ZW_ADD_NODE_TO_MASK(n, nms.cached_node_list);
    TEST_ASSERT_EQUAL(n + ZW_MAX_NODES - ZW_LR_MIN_NODE_ID + 1,
                      zwave_network_management_get_network_size());
  }
}