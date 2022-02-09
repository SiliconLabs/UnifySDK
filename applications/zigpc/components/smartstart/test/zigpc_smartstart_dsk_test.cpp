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

#include <workaround.hpp>

#include <unity.h>

#include "zigpc_smartstart_int.hpp"

extern "C" {

#include "zigpc_gateway_mock.h"
#include "zigpc_common_zigbee.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp() {}

void tearDown() {}

/**
 * @brief Scenario: Sanity test
 *
 * Ensure EUI64 and InstallCode+CRC is parsed properly
 *
 */
void test_dsk_parsing_sanity(void)
{
  zigpc_dsk_install_code_t test_data = {
    "01-02-03-04-05-06-07-08-01-02-03-04-05-06-07-08-AA-BB",
    10,
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0xAA, 0xBB},
  };

  // ARRANGE
  zigpc_gateway_install_code_is_valid_ExpectAndReturn(
    test_data.install_code,
    test_data.install_code_length,
    true);

  // ACT
  zigpc_dsk_install_code_t parsed_dsk;
  sl_status_t status
    = zigpc_smartstart_dsk_parse_install_code(test_data.dsk, parsed_dsk);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(test_data.eui64,
                               parsed_dsk.eui64,
                               sizeof(zigbee_eui64_t));
  TEST_ASSERT_EQUAL(test_data.install_code_length,
                    parsed_dsk.install_code_length);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(test_data.install_code,
                               parsed_dsk.install_code,
                               sizeof(uint8_t) * test_data.install_code_length);
}

/**
 * @brief Scenario: Empty DSK to be parsed.
 *
 * Parsing should fail since dsk length is not sufficient to get any
 * information.
 */
void test_dsk_parsing_dsk_empty(void)
{
  zigpc_dsk_install_code_t test_data = {
    "",
  };

  // ARRANGE

  // ACT
  zigpc_dsk_install_code_t parsed_dsk;
  sl_status_t status
    = zigpc_smartstart_dsk_parse_install_code(test_data.dsk, parsed_dsk);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_FAIL, status);
}

/**
 * @brief Scenario: DSK Length less than EUI64 length
 *
 * Parsing should fail since dsk length is not sufficient to get any
 * information.
 */
void test_dsk_parsing_dsk_shorter_than_eui64(void)
{
  zigpc_dsk_install_code_t test_data = {
    "01-02-03-04-05-06-07",
  };

  // ARRANGE

  // ACT
  zigpc_dsk_install_code_t parsed_dsk;
  sl_status_t status
    = zigpc_smartstart_dsk_parse_install_code(test_data.dsk, parsed_dsk);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_FAIL, status);
}

/**
 * @brief Scenario: DSK Length is exactly EUI64 length
 *
 * Parsing should fail since install code is missing.
 */
void test_dsk_parsing_dsk_only_eui64(void)
{
  zigpc_dsk_install_code_t test_data = {
    "01-02-03-04-05-06-07-09",
  };

  // ARRANGE

  // ACT
  zigpc_dsk_install_code_t parsed_dsk;
  sl_status_t status
    = zigpc_smartstart_dsk_parse_install_code(test_data.dsk, parsed_dsk);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_FAIL, status);
}

/**
 * @brief Scenario: DSK has EUI64 but has an odd install code size.
 *
 * Parsing should fail since install format is invalid.
 */
void test_dsk_parsing_dsk_odd_install_code_size(void)
{
  zigpc_dsk_install_code_t test_data
    = {"01-02-03-04-05-06-07-09-01", 1, {0}, {0x01}};

  // ARRANGE
  zigpc_gateway_install_code_is_valid_ExpectAndReturn(
    test_data.install_code,
    test_data.install_code_length,
    false);

  // ACT
  zigpc_dsk_install_code_t parsed_dsk;
  sl_status_t status
    = zigpc_smartstart_dsk_parse_install_code(test_data.dsk, parsed_dsk);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_SIGNATURE, status);
}

/**
 * @brief Scenario: DSK has EUI64 but has an even install code size.
 *
 * Parsing should fail since install code format is invalid.
 */
void test_dsk_parsing_dsk_even_install_code_size_invalid(void)
{
  zigpc_dsk_install_code_t test_data
    = {"01-02-03-04-05-06-07-09-01-02", 2, {0}, {0x01, 0x02}};

  // ARRANGE
  zigpc_gateway_install_code_is_valid_ExpectAndReturn(
    test_data.install_code,
    test_data.install_code_length,
    false);

  // ACT
  zigpc_dsk_install_code_t parsed_dsk;
  sl_status_t status
    = zigpc_smartstart_dsk_parse_install_code(test_data.dsk, parsed_dsk);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_SIGNATURE, status);
}

/**
 * @brief Scenario: DSK Length at max install code size with valid format.
 *
 * Parsing should pass since install code format is valid.
 */
void test_dsk_parsing_dsk_max_install_code_size_valid(void)
{
  zigpc_dsk_install_code_t test_data = {
    "01-02-03-04-05-06-07-09-00-10-20-30-40-50-60-70-80-90-A0-B0-C0-D0-E0-F0-"
    "AA-BB",
    18,
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x09},
    {0x00,
     0x10,
     0x20,
     0x30,
     0x40,
     0x50,
     0x60,
     0x70,
     0x80,
     0x90,
     0xA0,
     0xB0,
     0xC0,
     0xD0,
     0xE0,
     0xF0,
     0xAA,
     0xBB},
  };

  // ARRANGE
  zigpc_gateway_install_code_is_valid_ExpectAndReturn(
    test_data.install_code,
    test_data.install_code_length,
    true);

  // ACT
  zigpc_dsk_install_code_t parsed_dsk;
  sl_status_t status
    = zigpc_smartstart_dsk_parse_install_code(test_data.dsk, parsed_dsk);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(test_data.eui64,
                               parsed_dsk.eui64,
                               sizeof(zigbee_eui64_t));
  TEST_ASSERT_EQUAL(test_data.install_code_length,
                    parsed_dsk.install_code_length);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(test_data.install_code,
                               parsed_dsk.install_code,
                               sizeof(uint8_t) * test_data.install_code_length);
}

/**
 * @brief Scenario: DSK Length longer than max install code size.
 *
 * Parsing should fail since install code format is valid.
 */
void test_dsk_parsing_dsk_longer_than_install_code_size(void)
{
  zigpc_dsk_install_code_t test_data = {
    "01-02-03-04-05-06-07-09-00-10-20-30-40-50-60-70-80-90-A0-B0-C0-D0-E0-F0-"
    "AA-BB-CC",
  };

  // ARRANGE

  // ACT
  zigpc_dsk_install_code_t parsed_dsk;
  sl_status_t status
    = zigpc_smartstart_dsk_parse_install_code(test_data.dsk, parsed_dsk);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_FAIL, status);
}

/**
 * @brief Scenario: DSK but without any dashes
 *
 * Parsing should fail due to invalid DSK format
 */
void test_dsk_parsing_invalid_format(void)
{
  zigpc_dsk_install_code_t test_data = {
    "01020304050607080102030405060708AABB",
  };

  // ARRANGE

  // ACT
  zigpc_dsk_install_code_t parsed_dsk;
  sl_status_t status
    = zigpc_smartstart_dsk_parse_install_code(test_data.dsk, parsed_dsk);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_FAIL, status);
}

/**
 * @brief Scenario: Valid DSK but in Z-wave DSK format
 *
 * Parsing should fail due to invalid DSK format
 */
void test_dsk_parsing_invalid_format_zwave(void)
{
  zigpc_dsk_install_code_t test_data = {
    "01020-30405-06070-80102-03040-50607-08AAB-BCCDD",
  };

  // ARRANGE

  // ACT
  zigpc_dsk_install_code_t parsed_dsk;
  sl_status_t status
    = zigpc_smartstart_dsk_parse_install_code(test_data.dsk, parsed_dsk);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_FAIL, status);
}

/**
 * @brief Scenario: DSK with EUI64 format invalid
 *
 * Parsing should fail due to invalid DSK format
 */
void test_dsk_parsing_invalid_eui64_format(void)
{
  zigpc_dsk_install_code_t test_data = {
    "01020304050607-08-01-02-03-04-05-06-07-08-AA-BB",
  };

  // ARRANGE

  // ACT
  zigpc_dsk_install_code_t parsed_dsk;
  sl_status_t status
    = zigpc_smartstart_dsk_parse_install_code(test_data.dsk, parsed_dsk);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_FAIL, status);
}

/**
 * @brief Scenario: DSK with install code format invalid
 *
 * Parsing should fail due to invalid DSK format
 */
void test_dsk_parsing_invalid_ic_format(void)
{
  zigpc_dsk_install_code_t test_data = {
    "01-02-03-04-05-06-07-08-01-02030405060708AABB",
  };

  // ARRANGE

  // ACT
  zigpc_dsk_install_code_t parsed_dsk;
  sl_status_t status
    = zigpc_smartstart_dsk_parse_install_code(test_data.dsk, parsed_dsk);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_FAIL, status);
}

}  // extern "C"
