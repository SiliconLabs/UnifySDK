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

void helper_expect_counts(size_t expected_count,
                          attribute_store_node_t parent_node,
                          attribute_store_type_t child_type)
{
  for (size_t i = 0; i < expected_count; i++) {
    attribute_store_get_node_child_by_type_ExpectAndReturn(parent_node,
                                                           child_type,
                                                           i,
                                                           i + 1);
  }
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    parent_node,
    child_type,
    expected_count,
    ATTRIBUTE_STORE_INVALID_NODE);
}

void test_zigpc_datastore_get_attribute_list_count_sanity(void)
{
  // ARRANGE
  size_t count;
  zigbee_eui64_t eui64             = "\x03\x05\xFF\x29\x34\x11\x4A\xBB";
  zigbee_eui64_uint_t eui64_i      = 0x0305FF2934114ABB;
  zigbee_endpoint_id_t endpoint_id = 15;
  zcl_cluster_id_t cluster_id      = 0x1234;
  size_t expected_count            = 3U;

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

  helper_expect_counts(expected_count,
                       MOCK_DEV_1_EP1_CL1_NODE_ID,
                       ZIGPC_DS_TYPE_CLUSTER_ATTRIBUTE);

  // ACT
  count = zigpc_datastore_get_attribute_count(eui64,
                                              endpoint_id,
                                              ZCL_CLUSTER_SERVER_SIDE,
                                              cluster_id);

  // ASSERT
  TEST_ASSERT_EQUAL(3, count);
}

void test_zigpc_datastore_get_attribute_list_count_no_cluster(void)
{
  // ARRANGE
  size_t count;
  zigbee_eui64_t eui64             = "\x03\x55\xFF\x29\x34\x11\x4A\xB4";
  zigbee_eui64_uint_t eui64_i      = 0x0355FF2934114AB4;
  zigbee_endpoint_id_t endpoint_id = 7;
  zcl_cluster_id_t cluster_id      = 0x1235;

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
  count = zigpc_datastore_get_attribute_count(eui64,
                                              endpoint_id,
                                              ZCL_CLUSTER_SERVER_SIDE,
                                              cluster_id);

  // ASSERT
  TEST_ASSERT_EQUAL(0, count);
}

/**
 * @brief Read attribute list sanity test
 *
 */
void test_zigpc_datastore_attribute_list_read(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_eui64_t eui64             = "\x44\x55\xFF\x29\x34\x11\x4A\xB5";
  zigbee_eui64_uint_t eui64_i      = 0x4455FF2934114AB5;
  zigbee_endpoint_id_t endpoint_id = 11;
  zcl_cluster_id_t cluster_id      = 0x1326;
  size_t expected_count            = 10;
  zcl_attribute_id_t attribute_list[expected_count];
  zcl_attribute_id_t expected_attribute_list[expected_count];

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

  helper_expect_counts(expected_count,
                       MOCK_DEV_1_EP1_CL1_NODE_ID,
                       ZIGPC_DS_TYPE_CLUSTER_ATTRIBUTE);

  attribute_store_node_exists_ExpectAndReturn(MOCK_DEV_1_EP1_NODE_ID, true);
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    MOCK_DEV_1_EP1_NODE_ID,
    ZIGPC_DS_TYPE_SERVER_CLUSTER,
    DESIRED_ATTRIBUTE,
    (uint8_t *)&cluster_id,
    sizeof(zcl_cluster_id_t),
    0,
    MOCK_DEV_1_EP1_CL1_NODE_ID);

  attribute_store_node_exists_ExpectAndReturn(MOCK_DEV_1_EP1_CL1_NODE_ID, true);
  for (size_t i = 0; i < expected_count; i++) {
    attribute_store_get_node_child_by_type_ExpectAndReturn(
      MOCK_DEV_1_EP1_CL1_NODE_ID,
      ZIGPC_DS_TYPE_CLUSTER_ATTRIBUTE,
      i,
      i + 11);

    attribute_store_get_node_attribute_value_ExpectAndReturn(
      i + 11,
      DESIRED_ATTRIBUTE,
      (uint8_t *)(attribute_list + i),
      NULL,
      SL_STATUS_OK);
    attribute_store_get_node_attribute_value_IgnoreArg_value_size();
    attribute_store_get_node_attribute_value_ReturnMemThruPtr_value(
      (uint8_t *)(expected_attribute_list + i),
      sizeof(zcl_attribute_id_t));
  }

  // ACT
  size_t actual_count = expected_count;
  status              = zigpc_datastore_read_attribute_list(eui64,
                                               endpoint_id,
                                               ZCL_CLUSTER_SERVER_SIDE,
                                               cluster_id,
                                               attribute_list,
                                               &actual_count);
  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(expected_count, actual_count);
  TEST_ASSERT_EQUAL_UINT16_ARRAY(expected_attribute_list,
                                 attribute_list,
                                 expected_count);
}
/**
 * @brief Write attribute list sanity test
 *
 */
void test_zigpc_datastore_attribute_list_write(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_eui64_t eui64             = "\x44\x55\xFF\x29\x34\x11\x4A\x88";
  zigbee_eui64_uint_t eui64_i      = 0x4455FF2934114A88;
  zigbee_endpoint_id_t endpoint_id = 17;
  zcl_cluster_id_t cluster_id      = 0x1323;
  size_t expected_count            = 5;
  zcl_attribute_id_t attribute_list[expected_count];

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

  // remove children expects
  for (size_t i = 0; i < expected_count; i++) {
    attribute_store_get_node_child_by_type_ExpectAndReturn(
      MOCK_DEV_1_EP1_CL1_NODE_ID,
      ZIGPC_DS_TYPE_CLUSTER_ATTRIBUTE,
      i,
      i + 20);
    attribute_store_delete_node_ExpectAndReturn(i + 20, SL_STATUS_OK);
  }
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    MOCK_DEV_1_EP1_CL1_NODE_ID,
    ZIGPC_DS_TYPE_CLUSTER_ATTRIBUTE,
    expected_count,
    ATTRIBUTE_STORE_INVALID_NODE);

  // check to see if list already exists
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    MOCK_DEV_1_EP1_CL1_NODE_ID,
    ZIGPC_DS_TYPE_CLUSTER_ATTRIBUTE,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);

  for (size_t i = 0; i < expected_count; i++) {
    attribute_list[i] = 0xFF00 + i;
    attribute_store_node_exists_ExpectAndReturn(MOCK_DEV_1_EP1_CL1_NODE_ID,
                                                true);
    attribute_store_get_node_child_by_value_ExpectAndReturn(
      MOCK_DEV_1_EP1_CL1_NODE_ID,
      ZIGPC_DS_TYPE_CLUSTER_ATTRIBUTE,
      DESIRED_ATTRIBUTE,
      (uint8_t *)(attribute_list + i),
      sizeof(zcl_attribute_id_t),
      0,
      ATTRIBUTE_STORE_INVALID_NODE);

    // expect calls to create attribute
    attribute_store_add_node_ExpectAndReturn(ZIGPC_DS_TYPE_CLUSTER_ATTRIBUTE,
                                             MOCK_DEV_1_EP1_CL1_NODE_ID,
                                             i + 20);
    attribute_store_set_node_attribute_value_ExpectAndReturn(
      i + 20,
      DESIRED_ATTRIBUTE,
      (uint8_t *)(attribute_list + i),
      sizeof(zcl_attribute_id_t),
      SL_STATUS_OK);
  }

  // ACT
  status = zigpc_datastore_write_attribute_list(eui64,
                                                endpoint_id,
                                                ZCL_CLUSTER_SERVER_SIDE,
                                                cluster_id,
                                                attribute_list,
                                                expected_count);
  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}