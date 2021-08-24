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
#include "attribute_store_defined_attribute_types.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

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

  TEST_ASSERT_EQUAL(OPERATING_MODE_NL, get_operating_mode(test_node_id));
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

  TEST_ASSERT_EQUAL(OPERATING_MODE_FL, get_operating_mode(test_node_id));
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

  TEST_ASSERT_EQUAL(OPERATING_MODE_FL, get_operating_mode(test_node_id));
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

  TEST_ASSERT_EQUAL(OPERATING_MODE_AL, get_operating_mode(test_node_id));
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

  TEST_ASSERT_EQUAL(OPERATING_MODE_UNKNOWN, get_operating_mode(test_node_id));
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

  TEST_ASSERT_EQUAL(OPERATING_MODE_UNKNOWN, get_operating_mode(test_node_id));
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

  TEST_ASSERT_EQUAL(OPERATING_MODE_UNKNOWN, get_operating_mode(test_node_id));
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

  TEST_ASSERT_EQUAL(OPERATING_MODE_UNKNOWN, get_operating_mode(test_node_id));
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
  attribute_store_read_value_ExpectAndReturn(test_protocol_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(zwave_protocol_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(&protocol,
                                                         sizeof(protocol));

  TEST_ASSERT_EQUAL(34, get_protocol(test_node_id));
}

static sl_status_t
  read_protocol_stub(attribute_store_node_t id,
                     attribute_store_node_value_state_t value_state,
                     const uint8_t *value,
                     uint8_t value_size,
                     int numCalls)
{
  zwave_protocol_t protocol_inclusion = *(const zwave_protocol_t *)value;
  TEST_ASSERT_EQUAL(protocol_inclusion, PROTOCOL_ZWAVE);
  return SL_STATUS_OK;
}

void test_store_protocol()
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
  attribute_store_set_node_attribute_value_Stub(read_protocol_stub);
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

  attribute_store_read_value_ExpectAndReturn(test_protocol_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(zwave_protocol_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();

  TEST_ASSERT_EQUAL(PROTOCOL_UNKNOWN, get_protocol(test_node_id));
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

  TEST_ASSERT_EQUAL(SL_STATUS_BUSY,
                    store_protocol(test_node_id, expected_protocol));
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
                                           test_node_id,
                                           test_protocol_node);

  attribute_store_set_reported_ExpectWithArrayAndReturn(
    test_protocol_node,
    &expected_protocol,
    sizeof(expected_protocol),
    sizeof(zwave_protocol_t),
    SL_STATUS_OK);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    store_protocol(test_node_id, expected_protocol));
}

void test_get_protocol_name()
{
  TEST_ASSERT_EQUAL_STRING("PROTOCOL_ZWAVE",
                           zwave_get_protocol_name(PROTOCOL_ZWAVE));
  TEST_ASSERT_EQUAL_STRING("PROTOCOL_UNKNOWN",
                           zwave_get_protocol_name(PROTOCOL_UNKNOWN));
  TEST_ASSERT_EQUAL_STRING("PROTOCOL_ZWAVE_LONG_RANGE",
                           zwave_get_protocol_name(PROTOCOL_ZWAVE_LONG_RANGE));
  TEST_ASSERT_EQUAL_STRING("255", zwave_get_protocol_name(255));
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
