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
#include "zwave_command_class_device_reset_locally.h"
#include "zwave_command_class_test_helper.h"

#include "unity.h"
#include "ZW_classcmd.h"
#include "contiki_test_helper.h"
#include "zwave_controller_connection_info.h"

// Mocks
#include "zwave_controller_utils_mock.h"
#include "zwave_command_handler_mock.h"
#include "zwave_network_management_mock.h"
#include "zwave_controller_callbacks_mock.h"
#include "zwave_command_class_association_send_mock.h"

// UIC includes
#include "attribute_store.h"
#include "attribute_store_fixt.h"
#include "datastore.h"

//Generic includes
#include <string.h>

static zwave_controller_callbacks_t controller_callbacks = {};
static zwave_controller_reset_step_t reset_step          = NULL;
static on_group_send_complete_t lifeline_send_complete   = NULL;

// Environment constants
const zwave_home_id_t my_home_id = 0xcafecafe;
const zwave_node_id_t my_node_id = 3;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  //memset(&controller_callbacks, 0, sizeof(controller_callbacks));
  datastore_init(":memory:");
  attribute_store_init();

  // Ensure we start from scratch before creating our test network.
  attribute_store_delete_node(attribute_store_get_root());
  zwave_network_management_get_home_id_IgnoreAndReturn(my_home_id);
  zwave_network_management_get_node_id_IgnoreAndReturn(my_node_id);
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

static sl_status_t
  controller_callbacks_save(const zwave_controller_callbacks_t *cb, int n)
{
  controller_callbacks = *cb;
  return SL_STATUS_OK;
}
static sl_status_t
  controller_reset_step_save(zwave_controller_reset_step_t step_function,
                             int32_t priority,
                             int cmock_num_calls)
{
  TEST_ASSERT_EQUAL(1, priority);
  TEST_ASSERT_NOT_NULL(step_function);
  reset_step = step_function;
  return SL_STATUS_OK;
}

static sl_status_t
  zwave_association_send_save(association_group_id_t group_id,
                              uint16_t frame_size,
                              const uint8_t *frame,
                              bool supporting,
                              const zwave_tx_options_t *tx_options,
                              zwave_node_id_t excluded_node,
                              on_group_send_complete_t callback,
                              void *user,
                              int cmock_num_calls)
{
  TEST_ASSERT_NOT_NULL(callback);
  lifeline_send_complete = callback;
  return SL_STATUS_OK;
}

/// Called before each and every test
void setUp()
{
  reset_step             = NULL;
  lifeline_send_complete = NULL;
  contiki_test_helper_init();

  zwave_controller_register_callbacks_AddCallback(controller_callbacks_save);
  zwave_controller_register_callbacks_ExpectAndReturn(NULL, SL_STATUS_OK);
  zwave_controller_register_callbacks_IgnoreArg_callbacks();
  zwave_controller_register_reset_step_AddCallback(controller_reset_step_save);
  zwave_controller_register_reset_step_ExpectAndReturn(NULL, 1, SL_STATUS_OK);
  zwave_controller_register_reset_step_IgnoreArg_step_function();
  zwave_command_handler_register_handler_IgnoreAndReturn(SL_STATUS_OK);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_device_reset_locally_init());
}

void test_zwave_command_class_device_reset_locally_command()
{
  uint8_t device_reset_locally_cmd_frame[]
    = {COMMAND_CLASS_DEVICE_RESET_LOCALLY, DEVICE_RESET_LOCALLY_NOTIFICATION};
  zwave_controller_connection_info_t test_connection_info = {};
  test_connection_info.remote.node_id                     = 3;
  test_connection_info.remote.endpoint_id                 = 0;

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_device_reset_locally_control_handler(
                      &test_connection_info,
                      device_reset_locally_cmd_frame,
                      sizeof(device_reset_locally_cmd_frame)));

  zwave_network_management_get_state_ExpectAndReturn(0);
  zwave_network_management_remove_failed_ExpectAndReturn(
    test_connection_info.remote.node_id,
    SL_STATUS_OK);
  zwave_network_management_remove_failed_IgnoreArg_nodeid();

  contiki_test_helper_run(1000);

  TEST_ASSERT_NOT_NULL(controller_callbacks.on_node_deleted);
  controller_callbacks.on_node_deleted(test_connection_info.remote.node_id);
  contiki_test_helper_run(0);
}

void test_zwave_command_class_device_reset_locally_command_from_two_nodes()
{
  uint8_t device_reset_locally_cmd_frame[]
    = {COMMAND_CLASS_DEVICE_RESET_LOCALLY, DEVICE_RESET_LOCALLY_NOTIFICATION};
  zwave_controller_connection_info_t test_connection_info = {};
  test_connection_info.remote.node_id                     = 1;
  test_connection_info.remote.endpoint_id                 = 0;

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_device_reset_locally_control_handler(
                      &test_connection_info,
                      device_reset_locally_cmd_frame,
                      sizeof(device_reset_locally_cmd_frame)));

  contiki_test_helper_run(0);

  zwave_controller_connection_info_t test_connection_info_2 = {};
  test_connection_info_2.remote.node_id                     = 2;
  test_connection_info_2.remote.endpoint_id                 = 0;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_device_reset_locally_control_handler(
                      &test_connection_info_2,
                      device_reset_locally_cmd_frame,
                      sizeof(device_reset_locally_cmd_frame)));

  contiki_test_helper_run(0);

  // Remove the first node retry 1
  zwave_network_management_get_state_ExpectAndReturn(0);
  zwave_network_management_remove_failed_ExpectAndReturn(
    test_connection_info.remote.node_id,
    SL_STATUS_OK);
  contiki_test_helper_run(1000);
  // Remove the first node retry 2
  zwave_network_management_get_state_ExpectAndReturn(0);
  zwave_network_management_remove_failed_ExpectAndReturn(
    test_connection_info.remote.node_id,
    SL_STATUS_OK);
  contiki_test_helper_run(5000);
  // Remove the first node retry 3
  zwave_network_management_get_state_ExpectAndReturn(0);
  zwave_network_management_remove_failed_ExpectAndReturn(
    test_connection_info.remote.node_id,
    SL_STATUS_OK);
  contiki_test_helper_run(5000);

  //Remove the second node retry 1
  zwave_network_management_get_state_ExpectAndReturn(0);
  zwave_network_management_remove_failed_ExpectAndReturn(
    test_connection_info_2.remote.node_id,
    SL_STATUS_OK);
  contiki_test_helper_run(1000);
}

void test_zwave_command_class_device_reset_locally_lifeline_notification()
{
  TEST_ASSERT_NOT_NULL(reset_step);

  // Call the reset step
  const uint8_t expected_frame[] = {0x5A, 0x01};
  zwave_association_send_to_group_AddCallback(zwave_association_send_save);
  zwave_association_send_to_group_ExpectAndReturn(1,
                                                  sizeof(expected_frame),
                                                  expected_frame,
                                                  true,
                                                  NULL,
                                                  0,
                                                  NULL,
                                                  NULL,
                                                  SL_STATUS_OK);

  zwave_association_send_to_group_IgnoreArg_tx_options();
  zwave_association_send_to_group_IgnoreArg_callback();

  TEST_ASSERT_EQUAL(SL_STATUS_OK, reset_step());

  // Simulate a transmission OK. The next step of the reset should be invoked
  TEST_ASSERT_NOT_NULL(lifeline_send_complete);
  zwave_controller_on_reset_step_complete_Expect(1);
  lifeline_send_complete(SL_STATUS_OK, NULL);
}