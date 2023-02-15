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

#include <string.h>
#include <stdlib.h>
#include "unity.h"
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

void test_zigbee_eui64_to_uint_should_handle_invalid_input(void)
{
  // ARRANGE
  zigbee_eui64_uint_t result;

  // ACT
  result = zigbee_eui64_to_uint(NULL);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX64(0xFFFFFFFFFFFFFFFF, result);
}

void test_zigbee_eui64_to_uint_should_handle_valid_input(void)
{
  // ARRANGE
  zigbee_eui64_t eui64 = "\x0D\x02\x3F\x82\xA8\x6D\x30\x51";
  zigbee_eui64_uint_t result;

  // ACT
  result = zigbee_eui64_to_uint(eui64);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX64(0x0D023F82A86D3051, result);
}

void test_zigbee_uint_to_eui64_should_handle_invalid_input(void)
{
  // ARRANGE

  // ACT
  sl_status_t status = zigbee_uint_to_eui64(0x0, NULL);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
}

void test_zigbee_uint_to_eui64_should_handle_valid_input(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_eui64_uint_t src = 0x1023F4AC49003713;
  zigbee_eui64_t actual;
  zigbee_eui64_t expected = "\x10\x23\xF4\xAC\x49\x00\x37\x13";

  // ACT
  status = zigbee_uint_to_eui64(src, actual);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_MEMORY(expected, actual, sizeof(zigbee_eui64_t));
}

void test_zigbee_eui64_to_str_should_process_eui64(void)
{
  zigbee_eui64_t test_eui64 = {0x00, 0x01, 0x23, 0xAF, 0x00, 0x99, 0x3B, 0x00};
  char *expected_eui64      = "000123AF00993B00";
  char buffer[(ZIGBEE_EUI64_SIZE * 2) + 1];

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zigbee_eui64_to_str(test_eui64, buffer, sizeof(buffer)));
  TEST_ASSERT_EQUAL_STRING(expected_eui64, buffer);
}

void test_zigbee_eui64_to_str_should_process_zero_eui64(void)
{
  zigbee_eui64_t test_eui64 = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
  char *expected_eui64      = "0000000000000000";
  char buffer[(ZIGBEE_EUI64_SIZE * 2) + 1] = {0};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zigbee_eui64_to_str(test_eui64, buffer, sizeof(buffer)));
  TEST_ASSERT_EQUAL_STRING(expected_eui64, buffer);
}

void test_zigbee_eui64_to_str_should_reject_insufficient_buffer_lengths(void)
{
  zigbee_eui64_t test_eui64 = {0};
  char buffer[10] = {0};

  TEST_ASSERT_EQUAL(SL_STATUS_WOULD_OVERFLOW,
                    zigbee_eui64_to_str(test_eui64, buffer, sizeof(buffer)));
}

void test_is_valid_zcl_command(void)
{
  zcl_command_t command;

  command.cluster_id = 5;
  command.command_id = 5;

  bool is_valid = is_valid_zcl_command(command);

  TEST_ASSERT_EQUAL(true, is_valid);
}

void test_is_invalid_zcl_command(void)
{
  zcl_command_t command;

  command.cluster_id = ZCL_INVALID_CLUSTER_ID;
  command.command_id = ZCL_INVALID_COMMAND_ID;

  bool is_valid = is_valid_zcl_command(command);

  TEST_ASSERT_EQUAL(false, is_valid);
}

void test_is_valid_zcl_attribute(void)
{
  zcl_attribute_t attribute;

  attribute.cluster_id   = 5;
  attribute.attribute_id = 5;
  attribute.type         = ZCL_ATTR_TYPE_INT16;

  strncpy(attribute.desired_payload,
          "Some valid string",
          ZCL_DEFAULT_STR_LENGTH);
  strncpy(attribute.reported_payload,
          "Some valid string",
          ZCL_DEFAULT_STR_LENGTH);

  bool is_valid = is_valid_zcl_attribute(attribute);
  TEST_ASSERT_EQUAL(true, is_valid);
}

void test_is_invalid_zcl_attribute(void)
{
  zcl_attribute_t attribute = {
    .cluster_id       = ZCL_INVALID_CLUSTER_ID,
    .attribute_id     = ZCL_INVALID_ATTR_ID,
    .type             = ZCL_ATTR_TYPE_INVALID,
    .attribute_name   = "\0",
    .desired_payload  = "\0",
    .reported_payload = "\0",
  };

  bool is_valid = is_valid_zcl_attribute(attribute);
  TEST_ASSERT_FALSE(is_valid);
}

void test_is_valid_zcl_cluster(void)
{
  zcl_cluster_type_t cluster;

  cluster.cluster_id = 5;
  cluster.role       = ZCL_CLUSTER_SERVER_SIDE;

  bool is_valid = is_valid_zcl_cluster(cluster);
  TEST_ASSERT_EQUAL(true, is_valid);
}

void test_is_invalid_zcl_cluster(void)
{
  zcl_cluster_type_t cluster;

  cluster.cluster_id = ZCL_INVALID_CLUSTER_ID;

  bool is_valid = is_valid_zcl_cluster(cluster);
  TEST_ASSERT_EQUAL(false, is_valid);
}

void test_is_valid_endpoint(void)
{
  zigbee_endpoint_t endpoint;
  zcl_cluster_type_t cluster;

  cluster.cluster_id = 5;
  cluster.role       = ZCL_CLUSTER_SERVER_SIDE;

  bool is_valid = is_valid_zcl_cluster(cluster);
  TEST_ASSERT_EQUAL(true, is_valid);

  endpoint.cluster_list[0] = cluster;
  endpoint.cluster_count   = 1;
  endpoint.endpoint_id     = 1;

  is_valid = is_valid_zigbee_endpoint(endpoint);
  TEST_ASSERT_EQUAL(true, is_valid);

  endpoint.cluster_count = 0;
}

void test_is_valid_node(void)
{
  zigbee_node_t node;
  zigbee_endpoint_t endpoint;
  zcl_cluster_type_t cluster;

  cluster.cluster_id = 5;
  cluster.role       = ZCL_CLUSTER_SERVER_SIDE;

  bool is_valid = is_valid_zcl_cluster(cluster);
  TEST_ASSERT_EQUAL(true, is_valid);

  endpoint.cluster_list[0] = cluster;
  endpoint.cluster_count   = 1;
  endpoint.endpoint_id     = 1;

  is_valid = is_valid_zigbee_endpoint(endpoint);
  TEST_ASSERT_EQUAL(true, is_valid);

  zigbee_eui64_t eui64 = {0, 1, 2, 3, 4, 5, 6, 7};
  memcpy(node.eui64, eui64, 8 * sizeof(eui64[0]));
  node.endpoint_count = 1;
  node.is_active      = true;

  is_valid = is_valid_zigbee_node(node);
  TEST_ASSERT_EQUAL(true, is_valid);

  endpoint.cluster_count = 0;
  node.endpoint_count    = 0;
}

void test_copy_node_info(void)
{
  zcl_attribute_t attribute;
  zcl_command_t command;
  zigbee_node_t node;
  zigbee_endpoint_t endpoint;
  zcl_cluster_type_t cluster;

  command.cluster_id = 5;
  command.command_id = 5;

  attribute.cluster_id   = 5;
  attribute.attribute_id = 5;
  attribute.type         = ZCL_ATTR_TYPE_INT16;

  strncpy(attribute.desired_payload,
          "Some valid string",
          ZCL_DEFAULT_STR_LENGTH);
  strncpy(attribute.reported_payload,
          "Some valid string",
          ZCL_DEFAULT_STR_LENGTH);

  cluster.cluster_id = 5;
  cluster.role       = ZCL_CLUSTER_SERVER_SIDE;

  bool is_valid = is_valid_zcl_cluster(cluster);
  TEST_ASSERT_EQUAL(true, is_valid);

  endpoint.cluster_list[0] = cluster;
  endpoint.cluster_count   = 1;
  endpoint.endpoint_id     = 1;

  is_valid = is_valid_zigbee_endpoint(endpoint);
  TEST_ASSERT_EQUAL(true, is_valid);

  zigbee_eui64_t eui64 = {0, 1, 2, 3, 4, 5, 6, 7};
  memcpy(node.eui64, eui64, 8 * sizeof(eui64[0]));
  memcpy(&(node.endpoint_list[0]), &endpoint, sizeof(zigbee_endpoint_t));
  node.endpoint_count = 1;
  node.is_active      = true;

  is_valid = is_valid_zigbee_node(node);
  TEST_ASSERT_EQUAL(true, is_valid);

  zcl_command_t command_copy;
  zigbee_node_t node_copy;
  zigbee_endpoint_t endpoint_copy;

  sl_status_t status;

  status = copy_command(command, &command_copy);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(command.command_id, command_copy.command_id);
  TEST_ASSERT_EQUAL(command.cluster_id, command_copy.cluster_id);

  status = copy_endpoint(endpoint, &endpoint_copy);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);

  status = copy_node(node, &node_copy);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
}

void test_eui64_to_unid_invalid_args(void)
{
  // ARRANGE
  sl_status_t null_status;
  sl_status_t null_dest;
  zigbee_eui64_t eui64;

  // ACT
  null_status = zigpc_common_eui64_to_unid(NULL, NULL, 0);
  null_dest   = zigpc_common_eui64_to_unid(eui64, NULL, 0);
  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_NULL_POINTER, null_status);
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_NULL_POINTER, null_dest);
}

void test_eui64_to_unid_with_prefix(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_eui64_t test_eui64 = {0x0D, 0x0E, 0x0A, 0x0D, 0x0B, 0x9E, 0x0E, 0xFF};
  char *expected_dest       = "zb-0D0E0A0D0B9E0EFF";
  char test_dest[ZIGBEE_EUI64_HEX_STR_LENGTH + 3];

  // ACT
  status = zigpc_common_eui64_to_unid(test_eui64,
                                      test_dest,
                                      ZIGBEE_EUI64_HEX_STR_LENGTH + 3);
  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_STRING(expected_dest, test_dest);
}

void test_eui64_to_unid_insufficient_dest(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_eui64_t test_eui64 = {0x0D, 0x0E, 0x0A, 0x0D, 0x0B, 0x9E, 0x0E, 0xFF};
  char test_dest[ZIGBEE_EUI64_HEX_STR_LENGTH + 2];

  // ACT
  status = zigpc_common_eui64_to_unid(test_eui64,
                                      test_dest,
                                      ZIGBEE_EUI64_HEX_STR_LENGTH + 2);
  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_WOULD_OVERFLOW, status);
}
