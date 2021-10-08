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

#include "unity.h"

#include "attribute_store_mock.h"

#include "zigpc_datastore.h"
#include "zigpc_datastore_id.h"

#define MOCK_ROOT_NODE_ID  1
#define MOCK_NWK_NODE_ID   2
#define MOCK_DEV_1_NODE_ID 3

uint8_t default_network_index = PRIMARY_NETWORK_NUM;

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
void setUp(void)
{
  attribute_store_mock_Init();
}

/**
 * @brief Teardown after each test case
 *
 */
void tearDown(void)
{
  attribute_store_mock_Destroy();
}

void helper_assert_parent_set_overhead(attribute_store_node_t return_nodeid)
{
  attribute_store_get_root_ExpectAndReturn(MOCK_ROOT_NODE_ID);
  attribute_store_node_exists_ExpectAndReturn(MOCK_ROOT_NODE_ID, true);
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    MOCK_ROOT_NODE_ID,
    ZIGPC_DS_TYPE_NETWORK,
    DESIRED_ATTRIBUTE,
    (uint8_t *)&default_network_index,
    sizeof(uint8_t),
    0,
    return_nodeid);
}

void test_zigpc_datstore_device_count_sanity(void)
{
  // ARRANGE
  size_t count;
  size_t expected_count = 10U;

  helper_assert_parent_set_overhead(MOCK_NWK_NODE_ID);

  for (size_t i = 0; i < expected_count; i++) {
    attribute_store_get_node_child_by_type_ExpectAndReturn(MOCK_NWK_NODE_ID,
                                                           ZIGPC_DS_TYPE_DEVICE,
                                                           i,
                                                           i + 1);
  }
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    MOCK_NWK_NODE_ID,
    ZIGPC_DS_TYPE_DEVICE,
    expected_count,
    ATTRIBUTE_STORE_INVALID_NODE);

  // ACT
  count = zigpc_datastore_get_device_count();

  // ASSERT
  TEST_ASSERT_EQUAL(10, count);
}

void test_zigpc_datstore_device_count_no_network(void)
{
  // ARRANGE
  size_t count;

  helper_assert_parent_set_overhead(ATTRIBUTE_STORE_INVALID_NODE);

  // ACT
  count = zigpc_datastore_get_device_count();

  // ASSERT
  TEST_ASSERT_EQUAL(0, count);
}

void test_zigpc_datstore_device_find_by_index(void)
{
  // ARRANGE
  sl_status_t status;
  size_t index = 5;
  zigbee_eui64_t eui64;
  zigbee_eui64_uint_t expected_eui64 = 0x0102030405060708;
  zigbee_eui64_t expected_eui64_arr  = "\x01\x02\x03\x04\x05\x06\x07\x08";

  helper_assert_parent_set_overhead(MOCK_NWK_NODE_ID);

  attribute_store_get_node_child_by_type_ExpectAndReturn(MOCK_NWK_NODE_ID,
                                                         ZIGPC_DS_TYPE_DEVICE,
                                                         index,
                                                         MOCK_DEV_1_NODE_ID);

  attribute_store_get_node_attribute_value_ExpectAndReturn(MOCK_DEV_1_NODE_ID,
                                                           DESIRED_ATTRIBUTE,
                                                           NULL,
                                                           NULL,
                                                           SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnMemThruPtr_value(
    (uint8_t *)&expected_eui64,
    sizeof(zigbee_eui64_uint_t));

  // ACT
  status = zigpc_datastore_find_device_by_index(index, &eui64);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_eui64_arr,
                               eui64,
                               sizeof(zigbee_eui64_t));
}

void test_zigpc_datstore_device_find_by_index_invalid(void)
{
  // ARRANGE
  sl_status_t status;
  size_t index = 7;
  zigbee_eui64_t eui64;

  helper_assert_parent_set_overhead(MOCK_NWK_NODE_ID);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    MOCK_NWK_NODE_ID,
    ZIGPC_DS_TYPE_DEVICE,
    index,
    ATTRIBUTE_STORE_INVALID_NODE);

  // ACT
  status = zigpc_datastore_find_device_by_index(index, &eui64);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_NOT_FOUND, status);
}

void test_zigpc_datstore_device_create(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_eui64_t eui64_arr  = "\x0D\x0E\x0A\x0D\x0B\x0E\x0E\x0F";
  zigbee_eui64_uint_t eui64 = 0x0D0E0A0D0B0E0E0F;

  helper_assert_parent_set_overhead(MOCK_NWK_NODE_ID);

  attribute_store_node_exists_ExpectAndReturn(MOCK_NWK_NODE_ID, true);
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    MOCK_NWK_NODE_ID,
    ZIGPC_DS_TYPE_DEVICE,
    DESIRED_ATTRIBUTE,
    (uint8_t *)&eui64,
    sizeof(zigbee_eui64_uint_t),
    0,
    ATTRIBUTE_STORE_INVALID_NODE);

  // Expect calls to add nodes
  attribute_store_add_node_ExpectAndReturn(ZIGPC_DS_TYPE_DEVICE,
                                           MOCK_NWK_NODE_ID,
                                           MOCK_DEV_1_NODE_ID);
  attribute_store_set_node_attribute_value_ExpectAndReturn(
    MOCK_DEV_1_NODE_ID,
    DESIRED_ATTRIBUTE,
    (uint8_t *)&eui64,
    sizeof(zigbee_eui64_uint_t),
    SL_STATUS_OK);

  // ACT
  status = zigpc_datastore_create_device(eui64_arr);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}

void test_zigpc_datstore_device_read(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_eui64_uint_t eui64 = 0xF42783A3948723D8;
  zigbee_eui64_t eui64_arr  = "\xF4\x27\x83\xA3\x94\x87\x23\xD8";
  zigpc_device_data_t data;
  zigpc_device_data_t expected_data = {
    .network_status            = 1,
    .max_cmd_delay             = 1000,
    .endpoint_total_count      = 2,
    .endpoint_discovered_count = 1,
  };

  helper_assert_parent_set_overhead(MOCK_NWK_NODE_ID);

  attribute_store_node_exists_ExpectAndReturn(MOCK_NWK_NODE_ID, true);
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    MOCK_NWK_NODE_ID,
    ZIGPC_DS_TYPE_DEVICE,
    DESIRED_ATTRIBUTE,
    (uint8_t *)&eui64,
    sizeof(zigbee_eui64_uint_t),
    0,
    MOCK_DEV_1_NODE_ID);

  attribute_store_get_node_attribute_value_ExpectAndReturn(MOCK_DEV_1_NODE_ID,
                                                           REPORTED_ATTRIBUTE,
                                                           (uint8_t *)&data,
                                                           NULL,
                                                           SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnMemThruPtr_value(
    (uint8_t *)&expected_data,
    sizeof(zigpc_device_data_t));

  // ACT
  status = zigpc_datastore_read_device(eui64_arr, &data);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(expected_data.network_status, data.network_status);
  TEST_ASSERT_EQUAL(expected_data.max_cmd_delay, data.max_cmd_delay);
}

void test_zigpc_datstore_device_write(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_eui64_uint_t eui64 = 0xF42783A3948723D8;
  zigbee_eui64_t eui64_arr  = "\xF4\x27\x83\xA3\x94\x87\x23\xD8";
  zigpc_device_data_t data;

  helper_assert_parent_set_overhead(MOCK_NWK_NODE_ID);

  attribute_store_node_exists_ExpectAndReturn(MOCK_NWK_NODE_ID, true);
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    MOCK_NWK_NODE_ID,
    ZIGPC_DS_TYPE_DEVICE,
    DESIRED_ATTRIBUTE,
    (uint8_t *)&eui64,
    sizeof(zigbee_eui64_uint_t),
    0,
    MOCK_DEV_1_NODE_ID);

  attribute_store_set_node_attribute_value_ExpectAndReturn(
    MOCK_DEV_1_NODE_ID,
    REPORTED_ATTRIBUTE,
    (uint8_t *)&data,
    sizeof(zigpc_device_data_t),
    SL_STATUS_OK);

  // ACT
  status = zigpc_datastore_write_device(eui64_arr, &data);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}

void test_zigpc_datstore_device_delete(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_eui64_uint_t eui64 = 0xF42783A3948723D8;
  zigbee_eui64_t eui64_arr  = "\xF4\x27\x83\xA3\x94\x87\x23\xD8";

  helper_assert_parent_set_overhead(MOCK_NWK_NODE_ID);

  attribute_store_node_exists_ExpectAndReturn(MOCK_NWK_NODE_ID, true);
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    MOCK_NWK_NODE_ID,
    ZIGPC_DS_TYPE_DEVICE,
    DESIRED_ATTRIBUTE,
    (uint8_t *)&eui64,
    sizeof(zigbee_eui64_uint_t),
    0,
    MOCK_DEV_1_NODE_ID);

  attribute_store_delete_node_ExpectAndReturn(MOCK_DEV_1_NODE_ID, SL_STATUS_OK);

  // ACT
  status = zigpc_datastore_remove_device(eui64_arr);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}

void test_zigpc_datstore_device_remove_children(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_eui64_uint_t eui64 = 0xF42783A3948723D8;
  zigbee_eui64_t eui64_arr  = "\xF4\x27\x83\xA3\x94\x87\x23\xD8";
  size_t expected_children  = 5;

  {  // EXPECT
    helper_assert_parent_set_overhead(MOCK_NWK_NODE_ID);

    attribute_store_node_exists_ExpectAndReturn(MOCK_NWK_NODE_ID, true);
    attribute_store_get_node_child_by_value_ExpectAndReturn(
      MOCK_NWK_NODE_ID,
      ZIGPC_DS_TYPE_DEVICE,
      DESIRED_ATTRIBUTE,
      (uint8_t *)&eui64,
      sizeof(zigbee_eui64_uint_t),
      0,
      MOCK_DEV_1_NODE_ID);

    // remove_children expects
    attribute_store_node_exists_ExpectAndReturn(MOCK_NWK_NODE_ID, true);
    attribute_store_get_node_child_by_value_ExpectAndReturn(
      MOCK_NWK_NODE_ID,
      ZIGPC_DS_TYPE_DEVICE,
      DESIRED_ATTRIBUTE,
      (uint8_t *)&eui64,
      sizeof(zigbee_eui64_uint_t),
      0,
      MOCK_DEV_1_NODE_ID);

    attribute_store_get_node_child_count_ExpectAndReturn(MOCK_DEV_1_NODE_ID,
                                                         expected_children);
    for (size_t i = 0; i < expected_children; i++) {
      attribute_store_get_node_child_ExpectAndReturn(MOCK_DEV_1_NODE_ID,
                                                     i,
                                                     i + 10);
      attribute_store_delete_node_ExpectAndReturn(i + 10, SL_STATUS_OK);
    }
  }

  // ACT
  status = zigpc_datastore_remove_device_children(eui64_arr);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}
