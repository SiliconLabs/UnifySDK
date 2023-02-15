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
#include "unity.h"
#include "zwapi_protocol_transport.h"
#include "zwapi_func_ids.h"
#include "sl_log.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LOG_TAG "zwapi_protocol_transport_test"
// Mock includes
#include "zwapi_connection_mock.h"
#include "zwapi_session_mock.h"
#include "zwapi_init_mock.h"

// Static variables
static uint8_t response_length      = 0;
static uint8_t response_buffer[255] = {0};

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void callback_function_test(uint8_t transmitt_status, zwapi_tx_report_t *report)
{}

void test_zwapi_send_data_bridge_happy_case()
{
  zwapi_get_library_type_IgnoreAndReturn(ZWAVE_LIBRARY_TYPE_CONTROLLER_BRIDGE);
  zwave_node_id_t destination_node_id = 5;
  const uint8_t data[]                = {1, 2, 3, 5};
  uint8_t payload_buffer[180]         = {0};
  uint8_t tx_options                  = 0;
  payload_buffer[0]                   = 255;
  payload_buffer[1]                   = 5;
  payload_buffer[2]                   = 4;
  payload_buffer[3]                   = 1;
  payload_buffer[4]                   = 2;
  payload_buffer[5]                   = 3;
  payload_buffer[6]                   = 5;
  payload_buffer[12]                  = 1;
  response_length                     = 3 + IDX_DATA;
  response_buffer[3]                  = ZW_COMMAND_RETURN_VALUE_TRUE;
  zwapi_session_send_frame_with_response_ExpectWithArrayAndReturn(
    FUNC_ID_ZW_SEND_DATA_BRIDGE,
    payload_buffer,
    sizeof(payload_buffer),
    13,
    response_buffer,
    sizeof(response_buffer),
    &response_length,
    sizeof(response_length),
    SL_STATUS_OK);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();

  zwapi_session_send_frame_with_response_ReturnMemThruPtr_response_buf(
    response_buffer,
    response_length);
  zwapi_session_send_frame_with_response_ReturnThruPtr_response_len(
    &response_length);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwapi_send_data(destination_node_id,
                                    data,
                                    sizeof(data),
                                    tx_options,
                                    callback_function_test));
}

void test_zwapi_send_data_bridge_bad_case()
{
  zwapi_get_library_type_IgnoreAndReturn(ZWAVE_LIBRARY_TYPE_CONTROLLER_BRIDGE);

  //Return value false
  zwave_node_id_t destination_node_id = 5;
  const uint8_t data[]                = {1, 2, 3, 5};
  uint8_t payload_buffer[180]         = {0};
  uint8_t tx_options                  = 0;
  payload_buffer[0]                   = 255;
  payload_buffer[1]                   = 5;
  payload_buffer[2]                   = 4;
  payload_buffer[3]                   = 1;
  payload_buffer[4]                   = 2;
  payload_buffer[5]                   = 3;
  payload_buffer[6]                   = 5;
  payload_buffer[12]                  = 1;
  response_length                     = 3 + IDX_DATA;
  response_buffer[3]                  = ZW_COMMAND_RETURN_VALUE_FALSE;
  zwapi_session_send_frame_with_response_ExpectWithArrayAndReturn(
    FUNC_ID_ZW_SEND_DATA_BRIDGE,
    payload_buffer,
    sizeof(payload_buffer),
    13,
    response_buffer,
    sizeof(response_buffer),
    &response_length,
    sizeof(response_length),
    SL_STATUS_OK);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();

  zwapi_session_send_frame_with_response_ReturnMemThruPtr_response_buf(
    response_buffer,
    response_length);
  zwapi_session_send_frame_with_response_ReturnThruPtr_response_len(
    &response_length);

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwapi_send_data(destination_node_id,
                                    data,
                                    sizeof(data),
                                    tx_options,
                                    callback_function_test));

  //Response buffer length smaller than 3
  response_length = 2;
  zwapi_session_send_frame_with_response_ExpectWithArrayAndReturn(
    FUNC_ID_ZW_SEND_DATA_BRIDGE,
    payload_buffer,
    sizeof(payload_buffer),
    13,
    response_buffer,
    sizeof(response_buffer),
    &response_length,
    sizeof(response_length),
    SL_STATUS_OK);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();

  zwapi_session_send_frame_with_response_ReturnMemThruPtr_response_buf(
    response_buffer,
    response_length);
  zwapi_session_send_frame_with_response_ReturnThruPtr_response_len(
    &response_length);

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwapi_send_data(destination_node_id,
                                    data,
                                    sizeof(data),
                                    tx_options,
                                    callback_function_test));

  //Send command status fail
  response_length = 2;
  zwapi_session_send_frame_with_response_IgnoreAndReturn(SL_STATUS_FAIL);

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwapi_send_data(destination_node_id,
                                    data,
                                    sizeof(data),
                                    tx_options,
                                    callback_function_test));
}

void test_zwapi_send_data_happy_case()
{
  zwapi_get_library_type_IgnoreAndReturn(ZWAVE_LIBRARY_TYPE_CONTROLLER_STATIC);

  zwave_node_id_t destination_node_id = 5;
  const uint8_t data[]                = {1, 2, 3, 5};
  uint8_t payload_buffer[180]         = {0};
  uint8_t tx_options                  = 0;
  payload_buffer[0]                   = 5;
  payload_buffer[1]                   = 4;
  payload_buffer[2]                   = 1;
  payload_buffer[3]                   = 2;
  payload_buffer[4]                   = 3;
  payload_buffer[5]                   = 5;
  payload_buffer[7]                   = 1;
  response_length                     = 3 + IDX_DATA;
  response_buffer[3]                  = ZW_COMMAND_RETURN_VALUE_TRUE;
  zwapi_session_send_frame_with_response_ExpectWithArrayAndReturn(
    FUNC_ID_ZW_SEND_DATA,
    payload_buffer,
    sizeof(payload_buffer),
    8,
    response_buffer,
    sizeof(response_buffer),
    &response_length,
    sizeof(response_length),
    SL_STATUS_OK);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();

  zwapi_session_send_frame_with_response_ReturnMemThruPtr_response_buf(
    response_buffer,
    response_length);
  zwapi_session_send_frame_with_response_ReturnThruPtr_response_len(
    &response_length);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwapi_send_data(destination_node_id,
                                    data,
                                    sizeof(data),
                                    tx_options,
                                    callback_function_test));
}

void test_zwapi_send_data_too_long_frame()
{
  zwapi_get_library_type_IgnoreAndReturn(ZWAVE_LIBRARY_TYPE_CONTROLLER_STATIC);

  zwave_node_id_t destination_node_id = 5;
  const uint8_t data[]                = {1, 2, 3, 5};
  uint8_t tx_options                  = 0;

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwapi_send_data(destination_node_id,
                                    data,
                                    179,
                                    tx_options,
                                    callback_function_test));
}

void test_zwapi_send_data_bad_cases()
{
  zwapi_get_library_type_IgnoreAndReturn(ZWAVE_LIBRARY_TYPE_CONTROLLER_STATIC);

  zwave_node_id_t destination_node_id = 5;
  const uint8_t data[]                = {1, 2, 3, 5};
  uint8_t payload_buffer[180]         = {0};
  uint8_t tx_options                  = 0;
  payload_buffer[0]                   = 5;
  payload_buffer[1]                   = 4;
  payload_buffer[2]                   = 1;
  payload_buffer[3]                   = 2;
  payload_buffer[4]                   = 3;
  payload_buffer[5]                   = 5;
  payload_buffer[7]                   = 1;
  response_length                     = 0;
  response_buffer[3]                  = ZW_COMMAND_RETURN_VALUE_TRUE;
  //No ACK frame
  zwapi_session_send_frame_with_response_ExpectWithArrayAndReturn(
    FUNC_ID_ZW_SEND_DATA,
    payload_buffer,
    sizeof(payload_buffer),
    8,
    response_buffer,
    sizeof(response_buffer),
    &response_length,
    sizeof(response_length),
    SL_STATUS_OK);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();

  zwapi_session_send_frame_with_response_ReturnMemThruPtr_response_buf(
    response_buffer,
    response_length);
  zwapi_session_send_frame_with_response_ReturnThruPtr_response_len(
    &response_length);

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwapi_send_data(destination_node_id,
                                    data,
                                    sizeof(data),
                                    tx_options,
                                    callback_function_test));
  //Module rejected trasmission
  response_buffer[3] = ZW_COMMAND_RETURN_VALUE_FALSE;
  response_length    = 4;
  zwapi_session_send_frame_with_response_ExpectWithArrayAndReturn(
    FUNC_ID_ZW_SEND_DATA,
    payload_buffer,
    sizeof(payload_buffer),
    8,
    response_buffer,
    sizeof(response_buffer),
    &response_length,
    sizeof(response_length),
    SL_STATUS_OK);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();

  zwapi_session_send_frame_with_response_ReturnMemThruPtr_response_buf(
    response_buffer,
    response_length);
  zwapi_session_send_frame_with_response_ReturnThruPtr_response_len(
    &response_length);

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwapi_send_data(destination_node_id,
                                    data,
                                    sizeof(data),
                                    tx_options,
                                    callback_function_test));

  response_buffer[3] = ZW_COMMAND_RETURN_VALUE_TRUE;
  response_length    = 4;
  zwapi_session_send_frame_with_response_ExpectWithArrayAndReturn(
    FUNC_ID_ZW_SEND_DATA,
    payload_buffer,
    sizeof(payload_buffer),
    8,
    response_buffer,
    sizeof(response_buffer),
    &response_length,
    sizeof(response_length),
    SL_STATUS_FAIL);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();

  zwapi_session_send_frame_with_response_ReturnMemThruPtr_response_buf(
    response_buffer,
    response_length);
  zwapi_session_send_frame_with_response_ReturnThruPtr_response_len(
    &response_length);

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwapi_send_data(destination_node_id,
                                    data,
                                    sizeof(data),
                                    tx_options,
                                    callback_function_test));
}
