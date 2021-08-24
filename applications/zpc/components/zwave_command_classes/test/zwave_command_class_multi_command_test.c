/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "zwave_command_class_multi_command.h"

// Generic includes
#include <string.h>

// Test includes
#include "unity.h"

// Includes from other components
#include "ZW_classcmd.h"

// Mocks
#include "zwave_command_handler_mock.h"

// Typedefs
typedef struct zwave_multi_command_encapsulation_frame {
  uint8_t encapsulated_command_length;
  uint8_t encapsulated_command[ZWAVE_MAX_FRAME_SIZE];
} zwave_multi_command_encapsulation_frame_t;

// Static variables
static zwave_command_handler_t multi_command_handler                    = {};
static zwave_controller_connection_info_t last_received_connection_info = {};

static zwave_multi_command_encapsulation_frame_t received_frame[3] = {};
static uint8_t frame_counter                                       = 0;
static uint8_t happy_case                                          = 0;

sl_status_t zwave_command_handler_register_handler_stub(
  const zwave_command_handler_t handler, int cmock_num_calls)
{
  TEST_ASSERT_EQUAL(COMMAND_CLASS_MULTI_CMD, handler.command_class);
  TEST_ASSERT_EQUAL(MULTI_CMD_VERSION, handler.version);
  TEST_ASSERT_NOT_NULL(handler.support_handler);
  TEST_ASSERT_NULL(handler.control_handler);

  // Save the handler for our tests
  multi_command_handler = handler;

  return SL_STATUS_OK;
}

sl_status_t zwave_command_handler_dispatch_stub(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length,
  int cmock_num_calls)
{
  // Save the received data for test verification
  last_received_connection_info = *connection_info;

  memcpy(received_frame[frame_counter].encapsulated_command,
         frame_data,
         frame_length);
  received_frame[frame_counter++].encapsulated_command_length = frame_length;

  // Emulate SL_STATUS_FAIL for command 2
  if ((frame_counter == 2) && (happy_case == 0)) {
    return SL_STATUS_FAIL;
  } else {
    return SL_STATUS_OK;
  }
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
  happy_case    = 1;
  frame_counter = 0;
}

// Keep this test first
void test_zwave_command_class_multi_command_init()
{
  // Nothing to test here really. We intercept the registered handler.
  zwave_command_handler_register_handler_AddCallback(
    zwave_command_handler_register_handler_stub);
  zwave_command_handler_register_handler_ExpectAndReturn(multi_command_handler,
                                                         SL_STATUS_OK);
  zwave_command_handler_register_handler_IgnoreArg_new_command_class_handler();
  // Call the function
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_multi_command_init());
}

void test_decapsulation_happy_case()
{
  TEST_ASSERT_NOT_NULL(multi_command_handler.support_handler);

  zwave_controller_connection_info_t connection_info = {};
  const uint8_t frame_data[]
    = {COMMAND_CLASS_MULTI_CMD,
       MULTI_CMD_ENCAP,
       0x02,  // 2 CMDs
       0x11,  // Command Length 1 17 bytes
       COMMAND_CLASS_DOOR_LOCK,
       0x08,  // DOOR_LOCK_CAPABILITIES_REPORT
       0x06,  // SUPPORTED_OPERATION_TYPE_BIT_MASK_LENGTH
       0xde,
       0xad,
       0xbe,
       0xef,
       0xca,
       0xfe,  // SUPPORTED_OPERATION_TYPE_BIT_MASK
       0x05,  // SUPPORTED_DOOR_LOCK_MODE_LIST_LENGTH
       0x11,
       0x12,
       0x13,
       0x14,
       0x15,  // SUPPORTED_DOOR_LOCK_MODE
       0xcb,  // INSIDE_HANDLE_MODES + OUTSIDE_HANDLE_MODES
       0x03,  // SUPPORTED_DOOR_COMPONENTS
       0x0b,  // Command Length 2 11 bytes
       COMMAND_CLASS_DOOR_LOCK,
       0x06,  // DOOR_LOCK_CONFIGURATION_REPORT
       0x07,  // CONFIGURATION_OPERATION_TYPE
       0xde,  // INSIDE_HANDLE_MODES | OUTSIDE_HANDLE_MODES
       0x05,  // LOCK_TIMEOUT_MINUTES
       0x11,  // LOCK_TIMEOUT_SECONDS
       0xcb,  // AUTO_RELOCK_TIME MSB
       0x03,  // AUTO_RELOCK_TIME LSB
       0x04,  // HOLD_AND_RELEASE_TIME MSB
       0x05,  // HOLD_AND_RELEASE_TIME LSB
       0x00};

  happy_case = 1;

  zwave_command_handler_dispatch_Stub(zwave_command_handler_dispatch_stub);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    multi_command_handler.support_handler(&connection_info,
                                                          frame_data,
                                                          sizeof(frame_data)));

  // Verify that our decapsulation worked
  TEST_ASSERT_EQUAL(17, received_frame[0].encapsulated_command_length);
  const uint8_t expected_frame_data_0[] = {COMMAND_CLASS_DOOR_LOCK,
                                           0x08,
                                           0x06,
                                           0xde,
                                           0xad,
                                           0xbe,
                                           0xef,
                                           0xca,
                                           0xfe,
                                           0x05,
                                           0x11,
                                           0x12,
                                           0x13,
                                           0x14,
                                           0x15,
                                           0xcb,
                                           0x03,
                                           0x0b};

  TEST_ASSERT_EQUAL_INT8_ARRAY(expected_frame_data_0,
                               received_frame[0].encapsulated_command,
                               received_frame[0].encapsulated_command_length);

  TEST_ASSERT_EQUAL(11, received_frame[1].encapsulated_command_length);
  const uint8_t expected_frame_data_1[] = {COMMAND_CLASS_DOOR_LOCK,
                                           0x06,
                                           0x07,
                                           0xde,
                                           0x05,
                                           0x11,
                                           0xcb,
                                           0x03,
                                           0x04,
                                           0x05,
                                           0x00};

  TEST_ASSERT_EQUAL_INT8_ARRAY(expected_frame_data_1,
                               received_frame[1].encapsulated_command,
                               received_frame[1].encapsulated_command_length);
}

void test_decapsulation_not_happy_case()
{
  TEST_ASSERT_NOT_NULL(multi_command_handler.support_handler);

  zwave_controller_connection_info_t connection_info = {};
  const uint8_t frame_data[]                         = {
    COMMAND_CLASS_MULTI_CMD,
    MULTI_CMD_ENCAP,
    0x03,  // 3 CMDs
    0x04,  // Command Length 1 4 bytes
    COMMAND_CLASS_DOOR_LOCK,
    0x08,
    0x06,
    0x07,
    0x05,  // Command Length 2 5 bytes
    COMMAND_CLASS_DOOR_LOCK,
    0x08,
    0x11,
    0xcb,
    0x03,
    0x07,  // Command Length 3 7 bytes
    COMMAND_CLASS_DOOR_LOCK,
    0x06,
    0x07,
    0xde,
    0x05,
    0x11,
    0xcb,
  };

  happy_case = 0;  // Command 1, 3 callback should return SL_STATUS_OK
                   // Command 2 callback should return    SL_STATUS_FAIL

  zwave_command_handler_dispatch_Stub(zwave_command_handler_dispatch_stub);

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    multi_command_handler.support_handler(&connection_info,
                                                          frame_data,
                                                          sizeof(frame_data)));
}

void test_decapsulation_too_short_frame()
{
  // Test with wrong command
  TEST_ASSERT_NOT_NULL(multi_command_handler.support_handler);

  zwave_controller_connection_info_t connection_info = {};
  const uint8_t frame_data[]
    = {COMMAND_CLASS_MULTI_CMD, MULTI_CMD_ENCAP, 0x01, 0x02, 0x03, 0x04, 0x05};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    multi_command_handler.support_handler(&connection_info, frame_data, 4));
}

void test_decapsulation_too_long_command()
{
  // Test with wrong command
  TEST_ASSERT_NOT_NULL(multi_command_handler.support_handler);

  zwave_controller_connection_info_t connection_info = {};
  const uint8_t frame_data[]                         = {COMMAND_CLASS_MULTI_CMD,
                                MULTI_CMD_ENCAP,
                                0x01,
                                200,  // length more than ZWAVE_MAX_FRAME_SIZE
                                0x03,
                                0x04,
                                0x05};

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    multi_command_handler.support_handler(&connection_info,
                                                          frame_data,
                                                          sizeof(frame_data)));
}

void test_decapsulation_command_longer_than_frame_1()
{
  // Test with wrong command
  TEST_ASSERT_NOT_NULL(multi_command_handler.support_handler);

  zwave_controller_connection_info_t connection_info = {};
  const uint8_t frame_data[]                         = {
    COMMAND_CLASS_MULTI_CMD,
    MULTI_CMD_ENCAP,
    0x01,
    154,  // length less than ZWAVE_MAX_FRAME_SIZE, but larger than the current frame
    0x03,
    0x04,
    0x05};

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    multi_command_handler.support_handler(&connection_info,
                                                          frame_data,
                                                          sizeof(frame_data)));
}

void test_decapsulation_command_longer_than_frame_2()
{
  // Test with wrong command
  TEST_ASSERT_NOT_NULL(multi_command_handler.support_handler);

  zwave_controller_connection_info_t connection_info = {};
  const uint8_t frame_data[]                         = {COMMAND_CLASS_MULTI_CMD,
                                MULTI_CMD_ENCAP,
                                0x02,  // 2 CMDs
                                4,  // Command Length 1 4 bytes
                                0x03,
                                0x04,
                                0x05,
                                0x06,
                                4,  // Command Length 2 4 bytes
                                0x01,
                                0x02,
                                0x03,
                                0x04};
  // Command 2 requires 4 bytes. Lets emulate 12 bytes only frame lenght
  // instead of 13
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    multi_command_handler.support_handler(&connection_info,
                                          frame_data,
                                          sizeof(frame_data) - 1));
}

void test_decapsulation_command_longer_than_frame()
{
  // Test with wrong command
  TEST_ASSERT_NOT_NULL(multi_command_handler.support_handler);

  zwave_controller_connection_info_t connection_info = {};
  const uint8_t frame_data[]                         = {COMMAND_CLASS_MULTI_CMD,
                                MULTI_CMD_ENCAP,
                                0x02,  // 2 CMDs
                                4,  // Command Length 1 4 bytes
                                0x03,
                                0x04,
                                0x05,
                                0x06,
                                4,  // Command Length 2 4 bytes
                                0x01,
                                0x02,
                                0x03,
                                0x04};
  // Command 2 requires 4 bytes. Lets emulate 10 bytes only frame lenght
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    multi_command_handler.support_handler(&connection_info, frame_data, 10));
}

void test_decapsulation_too_wrong_command()
{
  TEST_ASSERT_NOT_NULL(multi_command_handler.support_handler);

  zwave_controller_connection_info_t connection_info = {};
  const uint8_t frame_data[]                         = {COMMAND_CLASS_MULTI_CMD,
                                MULTI_CMD_ENCAP + 1,
                                0x01,
                                0x02,
                                0x03,
                                0x04,
                                0x05};

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    multi_command_handler.support_handler(&connection_info,
                                                          frame_data,
                                                          sizeof(frame_data)));
}

void test_decapsulation_too_wrong_command_class()
{
  TEST_ASSERT_NOT_NULL(multi_command_handler.support_handler);

  zwave_controller_connection_info_t connection_info = {};
  const uint8_t frame_data[] = {COMMAND_CLASS_MULTI_CMD + 1,
                                MULTI_CMD_ENCAP,
                                0x01,
                                0x02,
                                0x03,
                                0x04,
                                0x05};

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    multi_command_handler.support_handler(&connection_info,
                                                          frame_data,
                                                          sizeof(frame_data)));
}

void test_decapsulation_too_wrong_command_number()
{
  TEST_ASSERT_NOT_NULL(multi_command_handler.support_handler);

  zwave_controller_connection_info_t connection_info = {};
  const uint8_t frame_data[]                         = {COMMAND_CLASS_MULTI_CMD,
                                MULTI_CMD_ENCAP,
                                0x00,  // Should be > 0
                                0x02,
                                0x03,
                                0x04,
                                0x05};

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    multi_command_handler.support_handler(&connection_info,
                                                          frame_data,
                                                          sizeof(frame_data)));
}
