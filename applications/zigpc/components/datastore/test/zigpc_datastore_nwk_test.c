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

#define MOCK_ROOT_NODE_ID 1
#define MOCK_NWK_NODE_ID  2

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

void test_zigpc_datstore_create_network(void)
{
  // ARRANGE
  sl_status_t status;

  attribute_store_get_root_ExpectAndReturn(MOCK_ROOT_NODE_ID);
  attribute_store_node_exists_ExpectAndReturn(MOCK_ROOT_NODE_ID, true);

  attribute_store_get_node_child_by_value_ExpectAndReturn(
    MOCK_ROOT_NODE_ID,
    ZIGPC_DS_TYPE_NETWORK,
    DESIRED_ATTRIBUTE,
    (uint8_t *)&default_network_index,
    sizeof(uint8_t),
    0,
    ATTRIBUTE_STORE_INVALID_NODE);

  attribute_store_add_node_ExpectAndReturn(ZIGPC_DS_TYPE_NETWORK,
                                           MOCK_ROOT_NODE_ID,
                                           MOCK_NWK_NODE_ID);

  attribute_store_set_node_attribute_value_ExpectAndReturn(
    MOCK_NWK_NODE_ID,
    DESIRED_ATTRIBUTE,
    (uint8_t *)&default_network_index,
    sizeof(uint8_t),
    SL_STATUS_OK);

  // ACT
  status = zigpc_datastore_create_network();

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_datstore_create_network_already_exists(void)
{
  // ARRANGE
  sl_status_t status;

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

  // ACT
  status = zigpc_datastore_create_network();

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_ALREADY_EXISTS, status);
}

void test_zigpc_datstore_read_network(void)
{
  // ARRANGE
  sl_status_t status;
  zigpc_network_data_t nwk;
  zigpc_network_data_t expected_nwk = {
    .gateway_eui64 = "\x0D\x0E\00\x12\x8A\x99\xFE\x25",
    .panid         = 0x1234,
    .ext_panid     = {0},
    .radio_channel = 34,
  };

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

  attribute_store_get_node_attribute_value_ExpectAndReturn(MOCK_NWK_NODE_ID,
                                                           REPORTED_ATTRIBUTE,
                                                           (uint8_t *)&nwk,
                                                           NULL,
                                                           SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnMemThruPtr_value(
    (uint8_t *)&expected_nwk,
    sizeof(zigpc_network_data_t));

  // ACT
  status = zigpc_datastore_read_network(&nwk);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_nwk.gateway_eui64,
                               nwk.gateway_eui64,
                               sizeof(zigbee_eui64_t));
  TEST_ASSERT_EQUAL(expected_nwk.panid, nwk.panid);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_nwk.ext_panid,
                               nwk.ext_panid,
                               sizeof(zigbee_ext_panid_t));
  TEST_ASSERT_EQUAL(expected_nwk.radio_channel, nwk.radio_channel);
}

void test_zigpc_datstore_read_network_not_found(void)
{
  // ARRANGE
  sl_status_t status;
  zigpc_network_data_t nwk;

  attribute_store_get_root_ExpectAndReturn(MOCK_ROOT_NODE_ID);
  attribute_store_node_exists_ExpectAndReturn(MOCK_ROOT_NODE_ID, true);

  attribute_store_get_node_child_by_value_ExpectAndReturn(
    MOCK_ROOT_NODE_ID,
    ZIGPC_DS_TYPE_NETWORK,
    DESIRED_ATTRIBUTE,
    (uint8_t *)&default_network_index,
    sizeof(uint8_t),
    0,
    ATTRIBUTE_STORE_INVALID_NODE);

  // ACT
  status = zigpc_datastore_read_network(&nwk);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_NOT_FOUND, status);
}

void test_zigpc_datstore_write_network(void)
{
  // ARRANGE
  sl_status_t status;
  zigpc_network_data_t nwk;

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

  attribute_store_set_node_attribute_value_ExpectAndReturn(
    MOCK_NWK_NODE_ID,
    REPORTED_ATTRIBUTE,
    (uint8_t *)&nwk,
    sizeof(zigpc_network_data_t),
    SL_STATUS_OK);

  // ACT
  status = zigpc_datastore_write_network(&nwk);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}

void test_zigpc_datstore_delete_network(void)
{
  // ARRANGE
  sl_status_t status;

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
  attribute_store_delete_node_ExpectAndReturn(MOCK_NWK_NODE_ID, SL_STATUS_OK);

  // ACT
  status = zigpc_datastore_remove_network();

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}
