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

#define MOCK_ROOT_NODE_ID          1
#define MOCK_NWK_NODE_ID           2
#define MOCK_DEV_1_NODE_ID         3
#define MOCK_DEV_1_EP1_NODE_ID     4
#define MOCK_DEV_1_EP1_CL1_NODE_ID 5

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

void helper_set_cluster_parents_overhead(zigbee_eui64_uint_t *eui64_i,
                                         zigbee_endpoint_id_t *endpoint_id,
                                         attribute_store_node_t return_nodeid)
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
    MOCK_NWK_NODE_ID);

  attribute_store_node_exists_ExpectAndReturn(MOCK_NWK_NODE_ID, true);
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    MOCK_NWK_NODE_ID,
    ZIGPC_DS_TYPE_DEVICE,
    DESIRED_ATTRIBUTE,
    (uint8_t *)eui64_i,
    sizeof(zigbee_eui64_uint_t),
    0,
    MOCK_DEV_1_NODE_ID);

  attribute_store_node_exists_ExpectAndReturn(MOCK_DEV_1_NODE_ID, true);
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    MOCK_DEV_1_NODE_ID,
    ZIGPC_DS_TYPE_ENDPOINT,
    DESIRED_ATTRIBUTE,
    (uint8_t *)endpoint_id,
    sizeof(zigbee_endpoint_id_t),
    0,
    return_nodeid);
}

void test_zigpc_datastore_get_cluster_count_sanity(void)
{
  // ARRANGE
  size_t count;
  zigbee_eui64_t eui64             = "\x03\x05\xFF\x29\x34\x11\x4A\xBB";
  zigbee_eui64_uint_t eui64_i      = 0x0305FF2934114ABB;
  zigbee_endpoint_id_t endpoint_id = 15;
  size_t expected_count            = 3U;

  helper_set_cluster_parents_overhead(&eui64_i,
                                      &endpoint_id,
                                      MOCK_DEV_1_EP1_NODE_ID);

  for (size_t i = 0; i < expected_count; i++) {
    attribute_store_get_node_child_by_type_ExpectAndReturn(
      MOCK_DEV_1_EP1_NODE_ID,
      ZIGPC_DS_TYPE_SERVER_CLUSTER,
      i,
      i + 1);
  }
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    MOCK_DEV_1_EP1_NODE_ID,
    ZIGPC_DS_TYPE_SERVER_CLUSTER,
    expected_count,
    ATTRIBUTE_STORE_INVALID_NODE);

  // ACT
  count = zigpc_datastore_get_cluster_count(eui64,
                                            endpoint_id,
                                            ZCL_CLUSTER_SERVER_SIDE);

  // ASSERT
  TEST_ASSERT_EQUAL(3, count);
}

void test_zigpc_datastore_get_cluster_count_no_device(void)
{
  // ARRANGE
  size_t count;
  zigbee_eui64_t eui64             = "\x03\x55\xFF\x29\x34\x11\x4A\xB4";
  zigbee_eui64_uint_t eui64_i      = 0x0355FF2934114AB4;
  zigbee_endpoint_id_t endpoint_id = 7;

  helper_set_cluster_parents_overhead(&eui64_i,
                                      &endpoint_id,
                                      ATTRIBUTE_STORE_INVALID_NODE);

  // ACT
  count = zigpc_datastore_get_cluster_count(eui64,
                                            endpoint_id,
                                            ZCL_CLUSTER_SERVER_SIDE);

  // ASSERT
  TEST_ASSERT_EQUAL(0, count);
}

void test_zigpc_datastore_cluster_find_by_index(void)
{
  // ARRANGE
  sl_status_t status;
  size_t index                         = 3;
  zigbee_eui64_t eui64                 = "\x03\x55\xFF\x29\x34\x11\x4A\xB5";
  zigbee_eui64_uint_t eui64_i          = 0x0355FF2934114AB5;
  zigbee_endpoint_id_t endpoint_id     = 46;
  zcl_cluster_id_t expected_cluster_id = 0x0030;
  zcl_cluster_id_t cluster_id;

  helper_set_cluster_parents_overhead(&eui64_i,
                                      &endpoint_id,
                                      MOCK_DEV_1_EP1_NODE_ID);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    MOCK_DEV_1_EP1_NODE_ID,
    ZIGPC_DS_TYPE_SERVER_CLUSTER,
    index,
    MOCK_DEV_1_EP1_CL1_NODE_ID);

  attribute_store_get_node_attribute_value_ExpectAndReturn(
    MOCK_DEV_1_EP1_CL1_NODE_ID,
    DESIRED_ATTRIBUTE,
    (uint8_t *)&cluster_id,
    NULL,
    SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnMemThruPtr_value(
    (uint8_t *)&expected_cluster_id,
    sizeof(zcl_cluster_id_t));

  // ACT
  status = zigpc_datastore_find_cluster_by_index(eui64,
                                                 endpoint_id,
                                                 ZCL_CLUSTER_SERVER_SIDE,
                                                 index,
                                                 &cluster_id);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(expected_cluster_id, cluster_id);
}

void test_zigpc_datastore_contains_cluster_true(void)
{
  zigbee_eui64_t eui64             = "\x03\x55\xFF\x20\x34\x11\x4A\xBB";
  zigbee_eui64_uint_t eui64_i      = 0x0355FF2034114ABB;
  zigbee_endpoint_id_t endpoint_id = 6;
  zcl_cluster_id_t cluster_id      = 0x1234;

  // ARRANGE
  helper_set_cluster_parents_overhead(&eui64_i,
                                      &endpoint_id,
                                      MOCK_DEV_1_EP1_NODE_ID);

  attribute_store_node_exists_ExpectAndReturn(MOCK_DEV_1_EP1_NODE_ID, true);
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    MOCK_DEV_1_EP1_NODE_ID,
    ZIGPC_DS_TYPE_SERVER_CLUSTER,
    DESIRED_ATTRIBUTE,
    (uint8_t *)&cluster_id,
    sizeof(zcl_cluster_id_t),
    0,
    MOCK_DEV_1_EP1_CL1_NODE_ID);

  // ACT
  bool is_found = zigpc_datastore_contains_cluster(eui64,
                                                   endpoint_id,
                                                   ZCL_CLUSTER_SERVER_SIDE,
                                                   cluster_id);

  // ASSERT
  TEST_ASSERT_TRUE(is_found);
}

void test_zigpc_datastore_contains_cluster_false(void)
{
  zigbee_eui64_t eui64             = "\x03\x55\xFF\x20\x34\x11\x4A\xBB";
  zigbee_eui64_uint_t eui64_i      = 0x0355FF2034114ABB;
  zigbee_endpoint_id_t endpoint_id = 6;
  zcl_cluster_id_t cluster_id      = 0x1234;

  // ARRANGE
  helper_set_cluster_parents_overhead(&eui64_i,
                                      &endpoint_id,
                                      MOCK_DEV_1_EP1_NODE_ID);

  attribute_store_node_exists_ExpectAndReturn(MOCK_DEV_1_EP1_NODE_ID, true);
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    MOCK_DEV_1_EP1_NODE_ID,
    ZIGPC_DS_TYPE_SERVER_CLUSTER,
    DESIRED_ATTRIBUTE,
    (uint8_t *)&cluster_id,
    sizeof(zcl_cluster_id_t),
    0,
    ATTRIBUTE_STORE_INVALID_NODE);

  // ACT
  bool is_found = zigpc_datastore_contains_cluster(eui64,
                                                   endpoint_id,
                                                   ZCL_CLUSTER_SERVER_SIDE,
                                                   cluster_id);

  // ASSERT
  TEST_ASSERT_FALSE(is_found);
}

void test_zigpc_datastore_cluster_create(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_eui64_t eui64             = "\x44\x55\xFF\x29\x34\x11\x4A\xB5";
  zigbee_eui64_uint_t eui64_i      = 0x4455FF2934114AB5;
  zigbee_endpoint_id_t endpoint_id = 11;
  zcl_cluster_id_t cluster_id      = 0x3134;

  helper_set_cluster_parents_overhead(&eui64_i,
                                      &endpoint_id,
                                      MOCK_DEV_1_EP1_NODE_ID);

  attribute_store_node_exists_ExpectAndReturn(MOCK_DEV_1_EP1_NODE_ID, true);
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    MOCK_DEV_1_EP1_NODE_ID,
    ZIGPC_DS_TYPE_SERVER_CLUSTER,
    DESIRED_ATTRIBUTE,
    (uint8_t *)&cluster_id,
    sizeof(zcl_cluster_id_t),
    0,
    ATTRIBUTE_STORE_INVALID_NODE);

  // Expect calls to add nodes
  attribute_store_add_node_ExpectAndReturn(ZIGPC_DS_TYPE_SERVER_CLUSTER,
                                           MOCK_DEV_1_EP1_NODE_ID,
                                           MOCK_DEV_1_EP1_CL1_NODE_ID);
  attribute_store_set_node_attribute_value_ExpectAndReturn(
    MOCK_DEV_1_EP1_CL1_NODE_ID,
    DESIRED_ATTRIBUTE,
    (uint8_t *)&cluster_id,
    sizeof(zcl_cluster_id_t),
    SL_STATUS_OK);

  // ACT
  status = zigpc_datastore_create_cluster(eui64,
                                          endpoint_id,
                                          ZCL_CLUSTER_SERVER_SIDE,
                                          cluster_id);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}

void test_zigpc_datastore_cluster_read(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_eui64_t eui64             = "\x44\x55\xFF\x29\x34\x11\x4A\xB5";
  zigbee_eui64_uint_t eui64_i      = 0x4455FF2934114AB5;
  zigbee_endpoint_id_t endpoint_id = 11;
  zcl_cluster_id_t cluster_id      = 0x1324;
  zigpc_cluster_data_t data;
  zigpc_cluster_data_t expected_data = {
    .dummy = 13,
  };

  helper_set_cluster_parents_overhead(&eui64_i,
                                      &endpoint_id,
                                      MOCK_DEV_1_EP1_NODE_ID);

  attribute_store_node_exists_ExpectAndReturn(MOCK_DEV_1_EP1_NODE_ID, true);
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    MOCK_DEV_1_EP1_NODE_ID,
    ZIGPC_DS_TYPE_SERVER_CLUSTER,
    DESIRED_ATTRIBUTE,
    (uint8_t *)&cluster_id,
    sizeof(zcl_cluster_id_t),
    0,
    MOCK_DEV_1_EP1_CL1_NODE_ID);

  attribute_store_get_node_attribute_value_ExpectAndReturn(
    MOCK_DEV_1_EP1_CL1_NODE_ID,
    REPORTED_ATTRIBUTE,
    (uint8_t *)&data,
    NULL,
    SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_ReturnMemThruPtr_value(
    (uint8_t *)&expected_data,
    sizeof(zigpc_cluster_data_t));

  // ACT
  status = zigpc_datastore_read_cluster(eui64,
                                        endpoint_id,
                                        ZCL_CLUSTER_SERVER_SIDE,
                                        cluster_id,
                                        &data);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_MEMORY(&expected_data, &data, sizeof(zigpc_cluster_data_t));
}

void test_zigpc_datastore_cluster_write(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_eui64_t eui64             = "\x44\x55\xFF\xEE\x34\x11\x4A\xB5";
  zigbee_eui64_uint_t eui64_i      = 0x4455FFEE34114AB5;
  zigbee_endpoint_id_t endpoint_id = 15;
  zcl_cluster_id_t cluster_id      = 0xFE12;
  zigpc_cluster_data_t data;

  helper_set_cluster_parents_overhead(&eui64_i,
                                      &endpoint_id,
                                      MOCK_DEV_1_EP1_NODE_ID);

  attribute_store_node_exists_ExpectAndReturn(MOCK_DEV_1_EP1_NODE_ID, true);
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    MOCK_DEV_1_EP1_NODE_ID,
    ZIGPC_DS_TYPE_SERVER_CLUSTER,
    DESIRED_ATTRIBUTE,
    (uint8_t *)&cluster_id,
    sizeof(zcl_cluster_id_t),
    0,
    MOCK_DEV_1_EP1_CL1_NODE_ID);

  attribute_store_set_node_attribute_value_ExpectAndReturn(
    MOCK_DEV_1_EP1_CL1_NODE_ID,
    REPORTED_ATTRIBUTE,
    (uint8_t *)&data,
    sizeof(zigpc_cluster_data_t),
    SL_STATUS_OK);

  // ACT
  status = zigpc_datastore_write_cluster(eui64,
                                         endpoint_id,
                                         ZCL_CLUSTER_SERVER_SIDE,
                                         cluster_id,
                                         &data);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}

void test_zigpc_datastore_cluster_delete(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_eui64_t eui64             = "\x44\x55\xFF\xEE\x34\x11\x4A\xB5";
  zigbee_eui64_uint_t eui64_i      = 0x4455FFEE34114AB5;
  zigbee_endpoint_id_t endpoint_id = 15;
  zcl_cluster_id_t cluster_id      = 0x0262;

  helper_set_cluster_parents_overhead(&eui64_i,
                                      &endpoint_id,
                                      MOCK_DEV_1_EP1_NODE_ID);

  attribute_store_node_exists_ExpectAndReturn(MOCK_DEV_1_EP1_NODE_ID, true);
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    MOCK_DEV_1_EP1_NODE_ID,
    ZIGPC_DS_TYPE_SERVER_CLUSTER,
    DESIRED_ATTRIBUTE,
    (uint8_t *)&cluster_id,
    sizeof(zcl_cluster_id_t),
    0,
    MOCK_DEV_1_EP1_CL1_NODE_ID);

  attribute_store_delete_node_ExpectAndReturn(MOCK_DEV_1_EP1_CL1_NODE_ID,
                                              SL_STATUS_OK);

  // ACT
  status = zigpc_datastore_remove_cluster(eui64,
                                          endpoint_id,
                                          ZCL_CLUSTER_SERVER_SIDE,
                                          cluster_id);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}
