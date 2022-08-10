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

#include "zigpc_group_mgmt_test.hpp"

extern "C" {

// Shared Unify includes
#include <unity.h>
#include <sl_status.h>

// ZigPC includes
#include "zigpc_group_map_mock.h"

#include <zigpc_group_mgmt_mqtt.h>

/**
 * @brief Setup the test suite (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp(void) {}

/**
 * @brief Teardown the test suite (called once after all test_xxx functions are called)
 *
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/**
 * @brief Setup before each test case
 *
 */
void setUp(void) {}

/**
 * @brief Teardown after each test case
 *
 */
void tearDown(void) {}

void test_assemble_name_payload_sanity(void)
{
  zigpc_group_member_t group_member = {
    .eui64       = {0xDE, 0xAD, 0xBE, 0xAF, 0x02, 0x34, 0x7E, 0xAB},
    .endpoint_id = 3,
  };
  zigbee_group_id_t group_id = 0x1;
  bool publish_type_reported = false;
  size_t name_payload_size   = 128;
  char name_payload[name_payload_size];

  char expected_name[]      = "TestGroup";
  size_t expected_name_size = sizeof(expected_name) / sizeof(expected_name[0]);
  char expected_payload[]   = R"({"value":"TestGroup"})";

  // ARRANGE
  zigpc_group_map_retrieve_group_name_ExpectAndReturn(group_member,
                                                      group_id,
                                                      publish_type_reported,
                                                      NULL,
                                                      NULL,
                                                      SL_STATUS_OK);
  zigpc_group_map_retrieve_group_name_IgnoreArg_group_name();
  zigpc_group_map_retrieve_group_name_IgnoreArg_group_name_size();
  zigpc_group_map_retrieve_group_name_ReturnMemThruPtr_group_name(
    expected_name,
    sizeof(char) * expected_name_size);
  zigpc_group_map_retrieve_group_name_ReturnThruPtr_group_name_size(
    &expected_name_size);

  // ACT
  sl_status_t status
    = zigpc_group_mqtt_assemble_name_payload(group_member.eui64,
                                             group_member.endpoint_id,
                                             group_id,
                                             publish_type_reported,
                                             name_payload,
                                             &name_payload_size);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_STRING(expected_payload, name_payload);
}

void test_assemble_name_payload_empty_name(void)
{
  zigpc_group_member_t group_member = {
    .eui64       = {0xDE, 0xAD, 0xBE, 0xAF, 0x02, 0x34, 0x7E, 0xAB},
    .endpoint_id = 3,
  };
  zigbee_group_id_t group_id = 0x1;
  bool publish_type_reported = false;
  size_t name_payload_size   = 128;
  char name_payload[name_payload_size];

  char expected_name[]      = "";
  size_t expected_name_size = sizeof(expected_name) / sizeof(expected_name[0]);
  char expected_payload[]   = R"({"value":""})";

  // ARRANGE
  zigpc_group_map_retrieve_group_name_ExpectAndReturn(group_member,
                                                      group_id,
                                                      publish_type_reported,
                                                      NULL,
                                                      NULL,
                                                      SL_STATUS_EMPTY);
  zigpc_group_map_retrieve_group_name_IgnoreArg_group_name();
  zigpc_group_map_retrieve_group_name_IgnoreArg_group_name_size();
  zigpc_group_map_retrieve_group_name_ReturnMemThruPtr_group_name(
    expected_name,
    sizeof(char) * expected_name_size);
  zigpc_group_map_retrieve_group_name_ReturnThruPtr_group_name_size(
    &expected_name_size);

  // ACT
  sl_status_t status
    = zigpc_group_mqtt_assemble_name_payload(group_member.eui64,
                                             group_member.endpoint_id,
                                             group_id,
                                             publish_type_reported,
                                             name_payload,
                                             &name_payload_size);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_STRING(expected_payload, name_payload);
}

void test_assemble_name_payload_not_found(void)
{
  zigpc_group_member_t group_member = {
    .eui64       = {0xDE, 0xAD, 0xBE, 0xAF, 0x02, 0x34, 0x7E, 0xAB},
    .endpoint_id = 3,
  };
  zigbee_group_id_t group_id = 0x1;
  bool publish_type_reported = false;
  size_t name_payload_size   = 128;
  char name_payload[name_payload_size];

  char expected_payload[] = "";

  // ARRANGE
  zigpc_group_map_retrieve_group_name_ExpectAndReturn(group_member,
                                                      group_id,
                                                      publish_type_reported,
                                                      NULL,
                                                      NULL,
                                                      SL_STATUS_NOT_FOUND);
  zigpc_group_map_retrieve_group_name_IgnoreArg_group_name();
  zigpc_group_map_retrieve_group_name_IgnoreArg_group_name_size();

  // ACT
  sl_status_t status
    = zigpc_group_mqtt_assemble_name_payload(group_member.eui64,
                                             group_member.endpoint_id,
                                             group_id,
                                             publish_type_reported,
                                             name_payload,
                                             &name_payload_size);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_STRING(expected_payload, name_payload);
}

}  // extern "C"
