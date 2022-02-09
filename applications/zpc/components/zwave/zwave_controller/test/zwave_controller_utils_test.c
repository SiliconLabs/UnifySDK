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
// Includes from this component
#include "zwave_controller_utils.h"
#include "zwave_controller_command_class_indices.h"

// Includes from other components
#include "unity.h"
#include "sl_log.h"

// Mock includes

// Generic includes
#include <stdbool.h>

// Test constant
static uint8_t test_super_tricky_nif[] = {
  0xD2,
  0x09,
  0xFB,
  COMMAND_CLASS_CONTROL_MARK,  // This one will be ignored, as it is part of an extended CC
  0xBD,
  0xCB,
  0x92,
  0xA7,
  0xFF,
  0x01,
  0xEA,
  0xF9,
  0xFF,
  0x66,
  COMMAND_CLASS_CONTROL_MARK,
  0x9F,
  0x44,
  0x28,
  0x00,
  0xF2,
  0x4E,
  0xF6};  // Erroneous ending, in the middle of an Extended CC
static uint8_t test_super_tricky_nif_length = sizeof(test_super_tricky_nif);

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp() {}

void test_zwave_parse_nif()
{
  zwave_command_class_t supported_command_classes[ZWAVE_MAX_NIF_SIZE];
  zwave_command_class_t controlled_command_classes[ZWAVE_MAX_NIF_SIZE];
  size_t supported_command_classes_length  = 0;
  size_t controlled_command_classes_length = 0;

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_parse_nif(test_super_tricky_nif,
                                    test_super_tricky_nif_length,
                                    supported_command_classes,
                                    &supported_command_classes_length,
                                    controlled_command_classes,
                                    &controlled_command_classes_length));

  // Check the supported CCs
  TEST_ASSERT_EQUAL(11, supported_command_classes_length);
  TEST_ASSERT_EQUAL(test_super_tricky_nif[0], supported_command_classes[0]);
  TEST_ASSERT_EQUAL(test_super_tricky_nif[1], supported_command_classes[1]);
  TEST_ASSERT_EQUAL(
    ((test_super_tricky_nif[2] << 8) | test_super_tricky_nif[3]),
    supported_command_classes[2]);
  TEST_ASSERT_EQUAL(test_super_tricky_nif[4], supported_command_classes[3]);
  TEST_ASSERT_EQUAL(test_super_tricky_nif[5], supported_command_classes[4]);
  TEST_ASSERT_EQUAL(test_super_tricky_nif[6], supported_command_classes[5]);
  TEST_ASSERT_EQUAL(test_super_tricky_nif[6], supported_command_classes[5]);
  TEST_ASSERT_EQUAL(test_super_tricky_nif[7], supported_command_classes[6]);
  TEST_ASSERT_EQUAL(
    ((test_super_tricky_nif[8] << 8) | test_super_tricky_nif[9]),
    supported_command_classes[7]);

  TEST_ASSERT_EQUAL(test_super_tricky_nif[10], supported_command_classes[8]);
  TEST_ASSERT_EQUAL(
    ((test_super_tricky_nif[11] << 8) | test_super_tricky_nif[12]),
    supported_command_classes[9]);
  TEST_ASSERT_EQUAL(test_super_tricky_nif[13], supported_command_classes[10]);

  // Now check the controlled CCs
  TEST_ASSERT_EQUAL(5, controlled_command_classes_length);
  TEST_ASSERT_EQUAL(test_super_tricky_nif[15], controlled_command_classes[0]);
  TEST_ASSERT_EQUAL(test_super_tricky_nif[16], controlled_command_classes[1]);
  TEST_ASSERT_EQUAL(test_super_tricky_nif[17], controlled_command_classes[2]);
  TEST_ASSERT_EQUAL(test_super_tricky_nif[18], controlled_command_classes[3]);
  TEST_ASSERT_EQUAL(
    ((test_super_tricky_nif[19] << 8) | test_super_tricky_nif[20]),
    controlled_command_classes[4]);
}

void test_zwave_parse_nif_null_pointers()
{
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_parse_nif(test_super_tricky_nif,
                                    test_super_tricky_nif_length,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL));
}

void test_zwave_network_scheme_str()
{
  // clang-format off
  TEST_ASSERT_EQUAL_STRING("Network Scheme", zwave_network_scheme_str(ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME));
  TEST_ASSERT_EQUAL_STRING("Unencrypted", zwave_network_scheme_str(ZWAVE_CONTROLLER_ENCAPSULATION_NONE));
  TEST_ASSERT_EQUAL_STRING("Security Scheme 0", zwave_network_scheme_str(ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0));
  TEST_ASSERT_EQUAL_STRING("Security 2, unauthenticated", zwave_network_scheme_str(ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED));
  TEST_ASSERT_EQUAL_STRING("Security 2, authenticated", zwave_network_scheme_str(ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED));
  TEST_ASSERT_EQUAL_STRING("Security 2, access", zwave_network_scheme_str(ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS));
  TEST_ASSERT_EQUAL_STRING("Unknown", zwave_network_scheme_str(42));
  // clang-format on
}

void test_is_command_class_in_supported_list()
{
  // Test constant
  static uint8_t test_nif[]      = {0xD2,
                               0x09,
                               0xFB,
                               0x01,
                               0xBD,
                               0xCB,
                               0x92,
                               0xA7,
                               0xCA,
                               0x1A,
                               0xEA,
                               0x99,
                               0xC6,
                               0x66,
                               0x9F,
                               0xFF};
  static uint8_t test_nif_length = sizeof(test_nif);

  TEST_ASSERT_TRUE(
    is_command_class_in_supported_list(0x09, test_nif, test_nif_length));
}