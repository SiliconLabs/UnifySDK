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
#include "zwave_utils.h"
// Includes from other components
#include "unity.h"

// Mock includes
#include "attribute_store_mock.h"
#include "attribute_store_helper_mock.h"
#include "zpc_attribute_store_network_helper_mock.h"
#include "zwave_controller_utils_mock.h"

#include "attribute_store_defined_attribute_types.h"
#include "zwave_command_class_indices.h"
#include "zwave_unid.h"
/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  zwave_unid_set_home_id(0x00000000);
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_get_operating_mode_happy_case_NL()
{
  zwave_node_id_t test_node_id                        = 119;
  attribute_store_node_t test_node_id_node            = 1;
  attribute_store_node_t test_listening_protocol_node = 2;

  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
    0,
    test_listening_protocol_node);

  uint8_t listening_protocol = 0;
  attribute_store_read_value_ExpectAndReturn(test_listening_protocol_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(uint8_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &listening_protocol,
    sizeof(listening_protocol));

  attribute_store_node_t test_optional_protocol_node = 3;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL,
    0,
    test_optional_protocol_node);

  uint8_t optional_protocol = 0;
  attribute_store_read_value_ExpectAndReturn(test_optional_protocol_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(uint8_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &optional_protocol,
    sizeof(optional_protocol));

  TEST_ASSERT_EQUAL(OPERATING_MODE_NL, zwave_get_operating_mode(test_node_id));
}

void test_get_operating_mode_happy_case_FL_250ms()
{
  zwave_node_id_t test_node_id                        = 119;
  attribute_store_node_t test_node_id_node            = 1;
  attribute_store_node_t test_listening_protocol_node = 2;

  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
    0,
    test_listening_protocol_node);

  uint8_t listening_protocol = 0;
  attribute_store_read_value_ExpectAndReturn(test_listening_protocol_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(uint8_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &listening_protocol,
    sizeof(listening_protocol));

  attribute_store_node_t test_optional_protocol_node = 3;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL,
    0,
    test_optional_protocol_node);

  uint8_t optional_protocol
    = ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_SENSOR_250MS_MASK;
  attribute_store_read_value_ExpectAndReturn(test_optional_protocol_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(uint8_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &optional_protocol,
    sizeof(optional_protocol));

  TEST_ASSERT_EQUAL(OPERATING_MODE_FL, zwave_get_operating_mode(test_node_id));
}

void test_get_operating_mode_happy_case_FL_1000ms()
{
  zwave_node_id_t test_node_id                        = 119;
  attribute_store_node_t test_node_id_node            = 1;
  attribute_store_node_t test_listening_protocol_node = 2;

  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
    0,
    test_listening_protocol_node);

  uint8_t listening_protocol = 0;
  attribute_store_read_value_ExpectAndReturn(test_listening_protocol_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(uint8_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &listening_protocol,
    sizeof(listening_protocol));

  attribute_store_node_t test_optional_protocol_node = 3;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL,
    0,
    test_optional_protocol_node);

  uint8_t optional_protocol
    = ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_SENSOR_1000MS_MASK;
  attribute_store_read_value_ExpectAndReturn(test_optional_protocol_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(uint8_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &optional_protocol,
    sizeof(optional_protocol));

  TEST_ASSERT_EQUAL(OPERATING_MODE_FL, zwave_get_operating_mode(test_node_id));
}

void test_get_operating_mode_happy_case_AL()
{
  zwave_node_id_t test_node_id                        = 119;
  attribute_store_node_t test_node_id_node            = 1;
  attribute_store_node_t test_listening_protocol_node = 2;

  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
    0,
    test_listening_protocol_node);

  uint8_t listening_protocol
    = ZWAVE_NODE_INFO_LISTENING_PROTOCOL_LISTENING_MASK;
  attribute_store_read_value_ExpectAndReturn(test_listening_protocol_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(uint8_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &listening_protocol,
    sizeof(listening_protocol));

  TEST_ASSERT_EQUAL(OPERATING_MODE_AL, zwave_get_operating_mode(test_node_id));
}

void test_get_operating_mode_error_attribute_invalid_1()
{
  zwave_node_id_t test_node_id             = 119;
  attribute_store_node_t test_node_id_node = 1;
  attribute_store_node_t test_listening_protocol_node
    = ATTRIBUTE_STORE_INVALID_NODE;

  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
    0,
    test_listening_protocol_node);

  TEST_ASSERT_EQUAL(OPERATING_MODE_UNKNOWN,
                    zwave_get_operating_mode(test_node_id));
}

void test_get_operating_mode_error_attribute_invalid_2()
{
  zwave_node_id_t test_node_id                        = 119;
  attribute_store_node_t test_node_id_node            = 1;
  attribute_store_node_t test_listening_protocol_node = 2;

  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
    0,
    test_listening_protocol_node);

  attribute_store_read_value_ExpectAndReturn(test_listening_protocol_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(uint8_t),
                                             SL_STATUS_FAIL);
  attribute_store_read_value_IgnoreArg_read_value();

  TEST_ASSERT_EQUAL(OPERATING_MODE_UNKNOWN,
                    zwave_get_operating_mode(test_node_id));
}

void test_get_operating_mode_error_attribute_invalid_3()
{
  zwave_node_id_t test_node_id                        = 119;
  attribute_store_node_t test_node_id_node            = 1;
  attribute_store_node_t test_listening_protocol_node = 2;

  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
    0,
    test_listening_protocol_node);

  uint8_t listening_protocol = 0;
  attribute_store_read_value_ExpectAndReturn(test_listening_protocol_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(uint8_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &listening_protocol,
    sizeof(listening_protocol));

  attribute_store_node_t test_optional_protocol_node
    = ATTRIBUTE_STORE_INVALID_NODE;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL,
    0,
    test_optional_protocol_node);

  TEST_ASSERT_EQUAL(OPERATING_MODE_UNKNOWN,
                    zwave_get_operating_mode(test_node_id));
}

void test_get_operating_mode_error_attribute_invalid_4()
{
  zwave_node_id_t test_node_id                        = 119;
  attribute_store_node_t test_node_id_node            = 1;
  attribute_store_node_t test_listening_protocol_node = 2;

  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
    0,
    test_listening_protocol_node);

  uint8_t listening_protocol = 0;
  attribute_store_read_value_ExpectAndReturn(test_listening_protocol_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(uint8_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &listening_protocol,
    sizeof(listening_protocol));

  attribute_store_node_t test_optional_protocol_node = 3;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL,
    0,
    test_optional_protocol_node);

  attribute_store_read_value_ExpectAndReturn(test_optional_protocol_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(uint8_t),
                                             SL_STATUS_NOT_FOUND);
  attribute_store_read_value_IgnoreArg_read_value();

  TEST_ASSERT_EQUAL(OPERATING_MODE_UNKNOWN,
                    zwave_get_operating_mode(test_node_id));
}

void test_get_protocol()
{
  zwave_node_id_t test_node_id              = 1;
  attribute_store_node_t test_node_id_node  = 5;
  attribute_store_node_t test_protocol_node = 7;

  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
    0,
    test_protocol_node);

  zwave_protocol_t protocol = 34;
  attribute_store_get_desired_else_reported_ExpectAndReturn(
    test_protocol_node,
    NULL,
    sizeof(zwave_protocol_t),
    SL_STATUS_OK);
  attribute_store_get_desired_else_reported_IgnoreArg_value();
  attribute_store_get_desired_else_reported_ReturnMemThruPtr_value(
    &protocol,
    sizeof(protocol));

  TEST_ASSERT_EQUAL(34, zwave_get_inclusion_protocol(test_node_id));
}

void test_get_protocol_attribute_store_empty()
{
  zwave_node_id_t test_node_id              = 1;
  attribute_store_node_t test_node_id_node  = 5;
  attribute_store_node_t test_protocol_node = 7;

  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
    0,
    test_protocol_node);

  attribute_store_get_desired_else_reported_ExpectAndReturn(
    test_protocol_node,
    NULL,
    sizeof(zwave_protocol_t),
    SL_STATUS_OK);
  attribute_store_get_desired_else_reported_IgnoreArg_value();

  TEST_ASSERT_EQUAL(PROTOCOL_UNKNOWN,
                    zwave_get_inclusion_protocol(test_node_id));
}

void test_store_protocol_attribute_store_happy_case()
{
  zwave_node_id_t test_node_id              = 1;
  attribute_store_node_t test_node_id_node  = 5;
  attribute_store_node_t test_protocol_node = 7;
  zwave_protocol_t expected_protocol        = PROTOCOL_ZWAVE_LONG_RANGE;

  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
    0,
    test_protocol_node);

  attribute_store_set_reported_ExpectWithArrayAndReturn(
    test_protocol_node,
    &expected_protocol,
    sizeof(expected_protocol),
    sizeof(zwave_protocol_t),
    SL_STATUS_BUSY);

  TEST_ASSERT_EQUAL(
    SL_STATUS_BUSY,
    zwave_store_inclusion_protocol(test_node_id, expected_protocol));
}

void test_store_protocol_attribute_store_no_node()
{
  zwave_node_id_t test_node_id              = 1;
  attribute_store_node_t test_node_id_node  = 5;
  attribute_store_node_t test_protocol_node = 7;
  zwave_protocol_t expected_protocol        = PROTOCOL_ZWAVE;

  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);

  attribute_store_add_node_ExpectAndReturn(ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                                           test_node_id_node,
                                           test_protocol_node);

  attribute_store_set_reported_ExpectWithArrayAndReturn(
    test_protocol_node,
    &expected_protocol,
    sizeof(expected_protocol),
    sizeof(zwave_protocol_t),
    SL_STATUS_OK);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_store_inclusion_protocol(test_node_id, expected_protocol));
}

void test_get_protocol_name()
{
  TEST_ASSERT_EQUAL_STRING("PROTOCOL_ZWAVE",
                           zwave_get_protocol_name(PROTOCOL_ZWAVE));
  TEST_ASSERT_EQUAL_STRING("PROTOCOL_UNKNOWN",
                           zwave_get_protocol_name(PROTOCOL_UNKNOWN));
  TEST_ASSERT_EQUAL_STRING("PROTOCOL_ZWAVE_LONG_RANGE",
                           zwave_get_protocol_name(PROTOCOL_ZWAVE_LONG_RANGE));
  TEST_ASSERT_EQUAL_STRING("Unknown", zwave_get_protocol_name(255));
}

void test_nodemask_add_node()
{
  zwave_nodemask_t nodemask = {0};
  for (zwave_node_id_t n = ZW_MIN_NODE_ID; n <= ZW_LR_MAX_NODE_ID; n++) {
    ZW_ADD_NODE_TO_MASK(n, nodemask);

    if (n < ZW_MAX_NODES) {
      TEST_ASSERT_EQUAL(1, IS_ZWAVE_NODE_ID(n));
      TEST_ASSERT_EQUAL(0, IS_ZWAVE_EXTENDED_NODE_ID(n));
    } else if (n > ZW_LR_MIN_NODE_ID) {
      TEST_ASSERT_EQUAL(0, IS_ZWAVE_NODE_ID(n));
      TEST_ASSERT_EQUAL(1, IS_ZWAVE_EXTENDED_NODE_ID(n));
    }

    if (IS_ZWAVE_NODE_ID_VALID(n)) {
      TEST_ASSERT_EQUAL(1, ZW_IS_NODE_IN_MASK(n, nodemask));
    } else {
      TEST_ASSERT_EQUAL(0, ZW_IS_NODE_IN_MASK(n, nodemask));
    }
  }
  for (zwave_node_id_t n = ZW_MIN_NODE_ID; n < ZW_LR_MAX_NODE_ID; n++) {
    ZW_REMOVE_NODE_FROM_MASK(n, nodemask);
    TEST_ASSERT_EQUAL(0, ZW_IS_NODE_IN_MASK(n, nodemask));
  }
  TEST_ASSERT_EQUAL(0, IS_ZWAVE_NODE_ID_VALID(233));
  TEST_ASSERT_EQUAL(0, IS_ZWAVE_NODE_ID_VALID(0));
  TEST_ASSERT_EQUAL(1, IS_ZWAVE_NODE_ID_VALID(1));
  TEST_ASSERT_EQUAL(0, IS_ZWAVE_NODE_ID_VALID(4001));
  TEST_ASSERT_EQUAL(0, IS_ZWAVE_NODE_ID(4001));
  TEST_ASSERT_EQUAL(0, IS_ZWAVE_EXTENDED_NODE_ID(4001));
  TEST_ASSERT_EQUAL(1, IS_ZWAVE_NODE_ID(232));
  TEST_ASSERT_EQUAL(0, IS_ZWAVE_EXTENDED_NODE_ID(232));
}

static const zwave_node_id_t test_node_id              = 0x39;
static const zwave_endpoint_id_t test_endpoint_id      = 0x12;
static unid_t test_unid                                = "zw-00000000-0039";
static const attribute_store_node_t test_endpoint_node = 0xBB;
static const attribute_store_node_t test_non_secure_nif_node = 0x8B;
static const attribute_store_node_t test_secure_nif_node     = 0x72;

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

//////////////////////////////////////////////////////////////////////////////
// Tests for zwave_node_supports_command_class
//////////////////////////////////////////////////////////////////////////////
void test_zwave_node_supports_command_class_happy_case()
{
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
  is_command_class_in_supported_list_ExpectAndReturn(0xC6,
                                                     test_nif,
                                                     test_nif_length,
                                                     1);
  TEST_ASSERT_TRUE(
    zwave_node_supports_command_class(0xC6, test_node_id, test_endpoint_id));
}

void test_zwave_node_supports_command_class_second_nif()
{
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
  is_command_class_in_supported_list_ExpectAndReturn(
    0xC6,
    test_nif_tricky_two_bytes,
    test_nif_tricky_two_bytes_length,
    0);

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
  is_command_class_in_supported_list_ExpectAndReturn(0xC6,
                                                     test_nif,
                                                     test_nif_length,
                                                     1);
  TEST_ASSERT_TRUE(
    zwave_node_supports_command_class(0xC6, test_node_id, test_endpoint_id));
}

void test_zwave_node_supports_command_class_invalid_attributes()
{
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
  is_command_class_in_supported_list_ExpectAndReturn(
    0x01,
    test_nif_tricky_two_bytes,
    test_nif_tricky_two_bytes_length,
    0);

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
  is_command_class_in_supported_list_ExpectAndReturn(0x01,
                                                     test_nif,
                                                     test_nif_length,
                                                     0);

  TEST_ASSERT_FALSE(
    zwave_node_supports_command_class(0x01, test_node_id, test_endpoint_id));
}

void test_zwave_node_supports_command_class_extended()
{
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
  is_command_class_in_supported_list_ExpectAndReturn(0xF9FF,
                                                     test_nif,
                                                     test_nif_length,
                                                     0);

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
  is_command_class_in_supported_list_ExpectAndReturn(
    0xF9FF,
    test_nif_tricky_two_bytes,
    test_nif_tricky_two_bytes_length,
    1);
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

void test_zwave_security_validation_is_node_s2_capable()
{
  // True test:
  zwave_node_id_t test_node_id             = 45;
  attribute_store_node_t test_node_id_node = 234;
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);
  attribute_store_node_t test_s2_capable_node = 321;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_NODE_IS_S2_CAPABLE,
    0,
    test_s2_capable_node);
  TEST_ASSERT_TRUE(zwave_security_validation_is_node_s2_capable(test_node_id));

  // False test:
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_NODE_IS_S2_CAPABLE,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);
  TEST_ASSERT_FALSE(zwave_security_validation_is_node_s2_capable(test_node_id));
}

void test_endpoint_exists()
{
  // True test:
  zwave_node_id_t test_node_id             = 45;
  zwave_endpoint_id_t test_endpoint_id     = 15;
  attribute_store_node_t test_node_id_node = 2;
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);

  attribute_store_get_node_child_by_value_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ENDPOINT_ID,
    REPORTED_ATTRIBUTE,
    NULL,
    sizeof(test_endpoint_id),
    0,
    1);
  attribute_store_get_node_child_by_value_IgnoreArg_value();

  TEST_ASSERT_TRUE(endpoint_exists(test_node_id, test_endpoint_id));

  // False test :
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_node_id,
    test_node_id_node);

  attribute_store_get_node_child_by_value_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ENDPOINT_ID,
    REPORTED_ATTRIBUTE,
    NULL,
    sizeof(test_endpoint_id),
    0,
    ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_get_node_child_by_value_IgnoreArg_value();

  TEST_ASSERT_FALSE(endpoint_exists(test_node_id, test_endpoint_id));
}

void test_is_command_in_array()
{
  // Extended CC without command:
  const uint8_t command_list_1[] = {0xFF, 0x00};
  TEST_ASSERT_FALSE(
    is_command_in_array(0xFF, 0x00, command_list_1, sizeof(command_list_1)));

  // CC without command:
  const uint8_t command_list_2[] = {0x9F};
  TEST_ASSERT_FALSE(
    is_command_in_array(0x9F, 0x00, command_list_2, sizeof(command_list_2)));

  // Regular array with 1 extended CC:
  const uint8_t command_list_3[]
    = {0x26, 0x03, 0x71, 0x05, 0xFF, 0x12, 0x01, 0x01, 0x01};
  TEST_ASSERT_FALSE(
    is_command_in_array(0x26, 0x00, command_list_3, sizeof(command_list_3)));
  TEST_ASSERT_TRUE(
    is_command_in_array(0x26, 0x03, command_list_3, sizeof(command_list_3)));
  TEST_ASSERT_TRUE(
    is_command_in_array(0xFF12, 0x01, command_list_3, sizeof(command_list_3)));
  TEST_ASSERT_TRUE(
    is_command_in_array(0x01, 0x01, command_list_3, sizeof(command_list_3)));

  // Array with Commands > 0xFE, but they are not to be interpreted as extended CCs
  // Trailing Single byte at the end
  const uint8_t command_list_4[]
    = {0x26, 0x03, 0x71, 0xFF, 0x9F, 0x12, 0x01, 0x01, 0x02};
  TEST_ASSERT_TRUE(
    is_command_in_array(0x71, 0xFF, command_list_4, sizeof(command_list_4)));
  TEST_ASSERT_FALSE(
    is_command_in_array(0x03, 0x71, command_list_4, sizeof(command_list_4)));
  TEST_ASSERT_FALSE(
    is_command_in_array(0x01, 0x02, command_list_4, sizeof(command_list_4)));
  TEST_ASSERT_TRUE(
    is_command_in_array(0x9F, 0x12, command_list_4, sizeof(command_list_4)));
}


void zwave_node_supports_command_class_mock_helper() {
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
  is_command_class_in_supported_list_ExpectAndReturn(COMMAND_CLASS_SUPERVISION,
                                                     test_nif,
                                                     test_nif_length,
                                                     1);
}

void test_zwave_node_want_supervision_frame_no_defined_flag_happy_case() {
  // Set mock support of COMMAND_CLASS_SUPERVISION
  zwave_node_supports_command_class_mock_helper();

  attribute_store_network_helper_get_endpoint_node_IgnoreAndReturn(0);
  // Set child reported to SL_STATUS_FAIL so we can simulate no flag
  attribute_store_get_child_reported_IgnoreAndReturn(SL_STATUS_FAIL);
  TEST_ASSERT_TRUE(
    zwave_node_want_supervision_frame(test_node_id, test_endpoint_id));
}

void test_zwave_node_want_supervision_frame_flag_happy_case() {
  // Set mock support of COMMAND_CLASS_SUPERVISION
  zwave_node_supports_command_class_mock_helper();

  // Simulate flag == 1
  attribute_store_node_t endpoint_node = 12;
  uint8_t supervision_flag = 1;
  attribute_store_network_helper_get_endpoint_node_IgnoreAndReturn(endpoint_node);
  attribute_store_get_child_reported_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_SUPERVISION_ENABLED,
    NULL,
    sizeof(supervision_flag),
    SL_STATUS_OK);
  attribute_store_get_child_reported_IgnoreArg_value();
  attribute_store_get_child_reported_ReturnThruPtr_value(&supervision_flag);

  TEST_ASSERT_TRUE(
    zwave_node_want_supervision_frame(test_node_id, test_endpoint_id));
}

void test_zwave_node_want_supervision_frame_flag_false_happy_case() {
  // Set mock support of COMMAND_CLASS_SUPERVISION
  zwave_node_supports_command_class_mock_helper();

  // Simulate flag == 0
  attribute_store_node_t endpoint_node = 12;
  uint8_t supervision_flag = 0;
  attribute_store_network_helper_get_endpoint_node_IgnoreAndReturn(endpoint_node);
  attribute_store_get_child_reported_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_SUPERVISION_ENABLED,
    NULL,
    sizeof(supervision_flag),
    SL_STATUS_OK);
  attribute_store_get_child_reported_IgnoreArg_value();
  attribute_store_get_child_reported_ReturnThruPtr_value(&supervision_flag);

  TEST_ASSERT_FALSE(
    zwave_node_want_supervision_frame(test_node_id, test_endpoint_id));
}

void test_zwave_node_want_supervision_no_supervision_cc_in_nif_happy_case() {
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
  is_command_class_in_supported_list_ExpectAndReturn(COMMAND_CLASS_SUPERVISION,
                                                     test_nif,
                                                     test_nif_length,
                                                     false);
  attribute_store_get_node_child_by_type_IgnoreAndReturn(ATTRIBUTE_STORE_INVALID_NODE); 


  TEST_ASSERT_FALSE(
    zwave_node_want_supervision_frame(test_node_id, test_endpoint_id));
}