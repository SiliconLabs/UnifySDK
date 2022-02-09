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
#include "zwave_command_classes_utils.h"

// Test includes
#include "unity.h"
#include "zwave_command_class_test_helper.h"

// Mock includes
#include "zwave_tx_scheme_selector_mock.h"
#include "zwave_tx_mock.h"
#include "zpc_attribute_store_network_helper_mock.h"

static zwave_controller_connection_info_t test_connection_info = {};
static zwave_endpoint_id_t test_endpoint_id                    = 0;

const uint8_t *command    = NULL;
uint16_t command_length   = 0;
const uint8_t *exp_frame  = NULL;
uint16_t exp_frame_length = 0;

static sl_status_t data_is_equal_callback(
  const zwave_controller_connection_info_t *connection,
  uint16_t data_length,
  const uint8_t *data,
  const zwave_tx_options_t *tx_options,
  const on_zwave_tx_send_data_complete_t on_send_complete,
  void *user,
  zwave_tx_session_id_t *session,
  int cmock_num_calls)
{
  TEST_ASSERT_EQUAL_UINT16(exp_frame_length, data_length);
  TEST_ASSERT_EQUAL_MEMORY(exp_frame, data, data_length);
  return SL_STATUS_OK;
}

void execute_frame_expect_result(handler command_handler,
                                 const uint8_t *command_frame,
                                 const uint16_t command_frame_length,
                                 const sl_status_t expected_status)
{
  command        = command_frame;
  command_length = command_frame_length;
  sl_status_t result
    = command_handler(&test_connection_info, command, command_length);

  TEST_ASSERT_EQUAL(expected_status, result);
}

void execute_frame_expect_frame(handler command_handler,
                                const uint8_t *command_frame,
                                const uint16_t command_frame_length,
                                const uint8_t *expected_frame,
                                const uint16_t expected_frame_length,
                                const sl_status_t expected_status)
{
  exp_frame        = expected_frame;
  exp_frame_length = expected_frame_length;

  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY,
    0,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();

  zwave_tx_send_data_ExpectAndReturn(&test_connection_info,
                                     exp_frame_length,
                                     exp_frame,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_AddCallback(&data_is_equal_callback);

  execute_frame_expect_result(command_handler,
                              command_frame,
                              command_frame_length,
                              expected_status);
}

void expect_report_frame(const uint8_t *expected_frame,
                         const uint16_t expected_frame_length)
{
  exp_frame        = expected_frame;
  exp_frame_length = expected_frame_length;

  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY,
    0,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();

  zwave_tx_send_data_ExpectAndReturn(NULL,
                                     exp_frame_length,
                                     exp_frame,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_AddCallback(&data_is_equal_callback);
}

void assert_dissallow_multicast(handler command_handler, uint8_t cc_cmd[2])
{
  zwave_controller_connection_info_t connection_info = {};
  connection_info.local.is_multicast                 = true;
  sl_status_t result = command_handler(&connection_info, cc_cmd, 2);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, result);
}

void is_zwave_command_class_filtered_for_root_device_verification(
  zwave_command_class_t command_class, attribute_store_node_t updated_node)
{
  if (is_actuator_command_class(command_class)) {
    attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
      updated_node,
      NULL,
      NULL,
      SL_STATUS_OK);

    test_endpoint_id = 2;
    attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
    attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
    attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
      &test_endpoint_id);
  }
}