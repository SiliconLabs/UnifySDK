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

#include "sl_status.h"
#include "zpc_converters.h"
#include "unity.h"
#include "sl_log.h"
#include <stdbool.h>

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return 0;
}

// Test storing int in the datastore
void test_zpc_converters_dsk_short_entries()
{
  // Test short DSK entries
  zwave_dsk_t dsk       = {0};
  const char test_dsk[] = "1-2-3-4-5-6-7-8";
  zwave_dsk_t expected  = {0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8};
  zpc_converters_dsk_str_to_internal(test_dsk, dsk);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, dsk, 16);
}
void test_zpc_converters_dsk_long_entries()
{
  // Test DSK from https://confluence.silabs.com/display/UIC/Network+Management+Commands
  zwave_dsk_t dsk       = {0};
  const char test_dsk[] = "24859-64107-46202-12845-60475-62452-54892-59867";
  // clang-format off
  zwave_dsk_t expected = {0x61, 0x1b, 0xfa, 0x6b, 0xb4, 0x7a, 0x32, 0x2d,
                          0xec, 0x3b, 0xf3, 0xf4, 0xd6, 0x6c, 0xe9, 0xdb};
  // clang-format on
  zpc_converters_dsk_str_to_internal(test_dsk, dsk);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, dsk, 16);
}
void test_zpc_converters_dsk_mixed_size_entries()
{
  // Test DSK with mixed short and long numbers (and 0)
  zwave_dsk_t dsk       = {0};
  const char test_dsk[] = "24859-1-46202-0-60475-62452-54892-0";
  // clang-format off
  zwave_dsk_t expected = {0x61, 0x1b, 0x00, 0x01, 0xb4, 0x7a, 0x00, 0x00,
                          0xec, 0x3b, 0xf3, 0xf4, 0xd6, 0x6c, 0x00, 0x00};
  // clang-format on

  zpc_converters_dsk_str_to_internal(test_dsk, dsk);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, dsk, 16);
}
void test_zpc_converters_dsk_fewer_elements()
{
  // Test DSK shorter than 8 elements
  zwave_dsk_t dsk = {0};
  memset(dsk, 0xff, 16);
  const char test_dsk[] = "24859-1-46202-0-60475-62452";
  // clang-format off
  zwave_dsk_t expected = {0x61, 0x1b, 0x00, 0x01, 0xb4, 0x7a, 0x00, 0x00,
                          0xec, 0x3b, 0xf3, 0xf4, 0xff, 0xff, 0xff, 0xff};
  // clang-format on

  zpc_converters_dsk_str_to_internal(test_dsk, dsk);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, dsk, 16);
}
void test_zpc_converters_dsk_too_many_elements()
{
  // Test DSK shorter than 8 elements
  zwave_dsk_t dsk       = {0};
  const char test_dsk[] = "1-2-3-4-5-6-7-8-9";
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_WOULD_OVERFLOW,
                            zpc_converters_dsk_str_to_internal(test_dsk, dsk),
                            "Expect error code");
}
void test_zpc_converters_dsk_too_large_value()
{
  zwave_dsk_t dsk       = {0};
  const char test_dsk[] = "123456-2-3-4-5-6-7-8";
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_FAIL,
                            zpc_converters_dsk_str_to_internal(test_dsk, dsk),
                            "Expect error code");
}
void test_zpc_converters_dsk_invalid_characters()
{
  zwave_dsk_t dsk       = {0};
  const char test_dsk[] = "k-2-3-4-5-6-7-8";
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_FAIL,
                            zpc_converters_dsk_str_to_internal(test_dsk, dsk),
                            "Expect error code");
  const char test_dsk2[] = "100k-2-3-4-5-6-7-8";
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_FAIL,
                            zpc_converters_dsk_str_to_internal(test_dsk2, dsk),
                            "Expect error code");
}

void test_zpc_converters_str_to_dsk()
{
  // clang-format off
  zwave_dsk_t dsk = {0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0xff, 0xff,
                     0x00, 0x00, 0x90, 0x30, 0x00, 0x12, 0x55, 0x15};
  // clang-format on
  char str[DSK_STR_LEN];
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zpc_converters_dsk_to_str(dsk, str, sizeof(str)));
  TEST_ASSERT_EQUAL_STRING("00000-00256-00001-65535-00000-36912-00018-21781",
                           str);
}

void test_zpc_converters_str_to_dsk_str_too_short()
{
  // clang-format off
  zwave_dsk_t dsk = {0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0xff, 0xff,
                     0x00, 0x00, 0x90, 0x30, 0x00, 0x12, 0x55, 0x15};
  // clang-format on
  char str[DSK_STR_LEN];
  TEST_ASSERT_EQUAL(SL_STATUS_WOULD_OVERFLOW,
                    zpc_converters_dsk_to_str(dsk, str, 10));

  TEST_ASSERT_EQUAL(SL_STATUS_WOULD_OVERFLOW,
                    zpc_converters_dsk_to_str(dsk, str, DSK_STR_LEN - 1));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zpc_converters_dsk_to_str(dsk, str, DSK_STR_LEN));
}
