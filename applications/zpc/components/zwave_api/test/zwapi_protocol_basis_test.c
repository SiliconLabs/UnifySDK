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
#include "zwapi_protocol_basis.h"
#include "zwapi_func_ids.h"

// Session mock:
#include "zwapi_session_mock.h"
#include "zwapi_init_mock.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_zwapi_soft_reset()
{
  zwapi_set_awaiting_zwave_api_started_Ignore();
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwapi_soft_reset());
}

void test_zwapi_get_zw_protocol_status()
{
  //Happy case
  uint8_t res_buffer[]
    = {0x19, 0x01, 0x09, 0xb2, 0x53, 0x02, 0x01, 0x02, 0x07, 0x30};
  uint8_t response_length = 10;

  zwapi_session_send_frame_with_response_ExpectAndReturn(
    FUNC_ID_ZW_GET_PROTOCOL_STATUS,
    NULL,
    0,
    res_buffer,
    &response_length,
    SL_STATUS_OK);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();
  zwapi_session_send_frame_with_response_ReturnMemThruPtr_response_buf(
    res_buffer,
    response_length);
  zwapi_session_send_frame_with_response_ReturnThruPtr_response_len(
    &response_length);
  TEST_ASSERT_EQUAL(0xb2, zwapi_get_zw_protocol_status());

  //Bad case
  response_length = 2;
  zwapi_session_send_frame_with_response_ExpectAndReturn(
    FUNC_ID_ZW_GET_PROTOCOL_STATUS,
    NULL,
    0,
    res_buffer,
    &response_length,
    SL_STATUS_OK);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();
  zwapi_session_send_frame_with_response_ReturnMemThruPtr_response_buf(
    res_buffer,
    response_length);
  zwapi_session_send_frame_with_response_ReturnThruPtr_response_len(
    &response_length);
  TEST_ASSERT_EQUAL(0, zwapi_get_zw_protocol_status());
}

void test_zwapi_get_zw_library_type()
{
  //Happy case
  uint8_t res_buffer[]
    = {0x19, 0x01, 0x09, 0xb2, 0x53, 0x02, 0x01, 0x02, 0x07, 0x30};
  uint8_t response_length = 10;

  zwapi_session_send_frame_with_response_ExpectAndReturn(
    FUNC_ID_ZW_TYPE_LIBRARY,
    NULL,
    0,
    res_buffer,
    &response_length,
    SL_STATUS_OK);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();
  zwapi_session_send_frame_with_response_ReturnMemThruPtr_response_buf(
    res_buffer,
    response_length);
  zwapi_session_send_frame_with_response_ReturnThruPtr_response_len(
    &response_length);
  TEST_ASSERT_EQUAL(0xb2, zwapi_get_zw_library_type());

  //Bad case
  response_length = 2;
  zwapi_session_send_frame_with_response_ExpectAndReturn(
    FUNC_ID_ZW_TYPE_LIBRARY,
    NULL,
    0,
    res_buffer,
    &response_length,
    SL_STATUS_OK);
  zwapi_session_send_frame_with_response_IgnoreArg_response_buf();
  zwapi_session_send_frame_with_response_IgnoreArg_response_len();
  zwapi_session_send_frame_with_response_ReturnMemThruPtr_response_buf(
    res_buffer,
    response_length);
  zwapi_session_send_frame_with_response_ReturnThruPtr_response_len(
    &response_length);
  TEST_ASSERT_EQUAL(0, zwapi_get_zw_library_type());
}
