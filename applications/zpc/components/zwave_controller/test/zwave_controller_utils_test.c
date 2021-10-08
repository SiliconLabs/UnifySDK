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
// Includes from this component
#include "zwave_controller_utils.h"
#include "zwave_controller_command_class_indices.h"

// Includes from other components
#include "unity.h"
#include "sl_log.h"

// Mock includes
#include "zwave_unid_mock.h"
#include "attribute_store_mock.h"
#include "attribute_store_helper_mock.h"
#include "zpc_attribute_store_network_helper_mock.h"
#include "attribute_store_defined_attribute_types.h"

// Generic includes
#include <stdbool.h>

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

static uint8_t test_nif_tricky_two_bytes[] = {0xD2,
                                              0x09,
                                              0xFB,
                                              0xF1,
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
                                              0x9F,
                                              0x4E};
static uint8_t test_nif_tricky_two_bytes_length
  = sizeof(test_nif_tricky_two_bytes);

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

static const zwave_node_id_t test_node_id                    = 0x39;
static const zwave_endpoint_id_t test_endpoint_id            = 0x12;
static unid_t test_unid                                      = "Eiffel tower";
static const attribute_store_node_t test_endpoint_node       = 0xBB;
static const attribute_store_node_t test_non_secure_nif_node = 0x8B;
static const attribute_store_node_t test_secure_nif_node     = 0x72;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp() {}

//////////////////////////////////////////////////////////////////////////////
// Tests for zwave_node_supports_command_class
//////////////////////////////////////////////////////////////////////////////
void test_zwave_node_supports_command_class_happy_case()
{
  zwave_unid_from_node_id_Expect(test_node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_endpoint_id,
    test_endpoint_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_ZWAVE_NIF,
    0,
    test_non_secure_nif_node);

  attribute_store_get_node_attribute_value_ExpectAndReturn(
    test_non_secure_nif_node,
    REPORTED_ATTRIBUTE,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnMemThruPtr_value(
    test_nif,
    (uint8_t)sizeof(test_nif));
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &test_nif_length);

  TEST_ASSERT_TRUE(
    zwave_node_supports_command_class(0xC6, test_node_id, test_endpoint_id));
}

void test_zwave_node_supports_command_class_second_nif()
{
  zwave_unid_from_node_id_Expect(test_node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_endpoint_id,
    test_endpoint_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_ZWAVE_NIF,
    0,
    test_non_secure_nif_node);

  attribute_store_get_node_attribute_value_ExpectAndReturn(
    test_non_secure_nif_node,
    REPORTED_ATTRIBUTE,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnMemThruPtr_value(
    test_nif_tricky_two_bytes,
    sizeof(test_nif_tricky_two_bytes));
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &test_nif_tricky_two_bytes_length);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_ZWAVE_SECURE_NIF,
    0,
    test_secure_nif_node);

  attribute_store_get_node_attribute_value_ExpectAndReturn(test_secure_nif_node,
                                                           REPORTED_ATTRIBUTE,
                                                           NULL,
                                                           NULL,
                                                           SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnMemThruPtr_value(
    test_nif,
    sizeof(test_nif));
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &test_nif_length);

  TEST_ASSERT_TRUE(
    zwave_node_supports_command_class(0xC6, test_node_id, test_endpoint_id));
}

void test_zwave_node_supports_command_class_invalid_attributes()
{
  zwave_unid_from_node_id_Expect(test_node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_endpoint_id,
    test_endpoint_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_ZWAVE_NIF,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_ZWAVE_SECURE_NIF,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);

  TEST_ASSERT_FALSE(
    zwave_node_supports_command_class(0xC6, test_node_id, test_endpoint_id));
}

void test_zwave_node_supports_command_class_cant_read_attributes()
{
  zwave_unid_from_node_id_Expect(test_node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_endpoint_id,
    test_endpoint_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_ZWAVE_NIF,
    0,
    test_non_secure_nif_node);

  attribute_store_get_node_attribute_value_ExpectAndReturn(
    test_non_secure_nif_node,
    REPORTED_ATTRIBUTE,
    NULL,
    NULL,
    SL_STATUS_FAIL);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnMemThruPtr_value(
    test_nif_tricky_two_bytes,
    sizeof(test_nif_tricky_two_bytes));
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &test_nif_tricky_two_bytes_length);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_ZWAVE_SECURE_NIF,
    0,
    test_secure_nif_node);

  attribute_store_get_node_attribute_value_ExpectAndReturn(test_secure_nif_node,
                                                           REPORTED_ATTRIBUTE,
                                                           NULL,
                                                           NULL,
                                                           SL_STATUS_FAIL);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnMemThruPtr_value(
    test_nif,
    sizeof(test_nif));
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &test_nif_length);

  TEST_ASSERT_FALSE(
    zwave_node_supports_command_class(0xC6, test_node_id, test_endpoint_id));
}

void test_zwave_node_supports_command_class_in_no_list()
{
  zwave_unid_from_node_id_Expect(test_node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_endpoint_id,
    test_endpoint_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_ZWAVE_NIF,
    0,
    test_non_secure_nif_node);

  attribute_store_get_node_attribute_value_ExpectAndReturn(
    test_non_secure_nif_node,
    REPORTED_ATTRIBUTE,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnMemThruPtr_value(
    test_nif_tricky_two_bytes,
    sizeof(test_nif_tricky_two_bytes));
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &test_nif_tricky_two_bytes_length);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_ZWAVE_SECURE_NIF,
    0,
    test_secure_nif_node);

  attribute_store_get_node_attribute_value_ExpectAndReturn(test_secure_nif_node,
                                                           REPORTED_ATTRIBUTE,
                                                           NULL,
                                                           NULL,
                                                           SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnMemThruPtr_value(
    test_nif,
    sizeof(test_nif));
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &test_nif_length);

  TEST_ASSERT_FALSE(
    zwave_node_supports_command_class(0x01, test_node_id, test_endpoint_id));
}

void test_zwave_node_supports_command_class_extended()
{
  zwave_unid_from_node_id_Expect(test_node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_endpoint_id,
    test_endpoint_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_ZWAVE_NIF,
    0,
    test_non_secure_nif_node);

  attribute_store_get_node_attribute_value_ExpectAndReturn(
    test_non_secure_nif_node,
    REPORTED_ATTRIBUTE,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnMemThruPtr_value(
    test_nif,
    sizeof(test_nif));
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &test_nif_length);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_ZWAVE_SECURE_NIF,
    0,
    test_secure_nif_node);

  attribute_store_get_node_attribute_value_ExpectAndReturn(test_secure_nif_node,
                                                           REPORTED_ATTRIBUTE,
                                                           NULL,
                                                           NULL,
                                                           SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnMemThruPtr_value(
    test_nif_tricky_two_bytes,
    sizeof(test_nif_tricky_two_bytes));
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &test_nif_tricky_two_bytes_length);

  TEST_ASSERT_TRUE(
    zwave_node_supports_command_class(0xF9FF, test_node_id, test_endpoint_id));
}

//////////////////////////////////////////////////////////////////////////////
// Tests for zwave_node_get_command_class_version
//////////////////////////////////////////////////////////////////////////////
void test_zwave_node_get_command_class_version_happy_case()
{
  zwave_command_class_t command_class      = 0x9384;
  zwave_cc_version_t command_class_version = 23;
  attribute_store_node_t test_version_node = 3495;

  zwave_unid_from_node_id_Expect(test_node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_endpoint_id,
    test_endpoint_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ZWAVE_CC_VERSION_ATTRIBUTE(command_class),
    0,
    test_version_node);

  attribute_store_get_reported_ExpectAndReturn(test_version_node,
                                               NULL,
                                               sizeof(command_class_version),
                                               SL_STATUS_OK);
  attribute_store_get_reported_IgnoreArg_value();
  attribute_store_get_reported_ReturnMemThruPtr_value(
    &command_class_version,
    sizeof(command_class_version));

  TEST_ASSERT_EQUAL(command_class_version,
                    zwave_node_get_command_class_version(command_class,
                                                         test_node_id,
                                                         test_endpoint_id));
}

void test_zwave_node_get_command_class_version_unkonwn_node()
{
  zwave_command_class_t command_class = 0x9384;

  zwave_unid_from_node_id_Expect(test_node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_endpoint_id,
    ATTRIBUTE_STORE_INVALID_NODE);

  TEST_ASSERT_EQUAL(0,
                    zwave_node_get_command_class_version(command_class,
                                                         test_node_id,
                                                         test_endpoint_id));
}

void test_zwave_node_get_command_class_version_read_error()
{
  zwave_command_class_t command_class      = 0x9384;
  attribute_store_node_t test_version_node = 3495;

  zwave_unid_from_node_id_Expect(test_node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_endpoint_id,
    test_endpoint_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ZWAVE_CC_VERSION_ATTRIBUTE(command_class),
    0,
    test_version_node);

  attribute_store_get_reported_ExpectAndReturn(test_version_node,
                                               NULL,
                                               sizeof(zwave_cc_version_t),
                                               SL_STATUS_FAIL);
  attribute_store_get_reported_IgnoreArg_value();

  TEST_ASSERT_EQUAL(0,
                    zwave_node_get_command_class_version(command_class,
                                                         test_node_id,
                                                         test_endpoint_id));
}

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