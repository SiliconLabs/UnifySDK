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

#include "zigpc_gateway_datastore_workaround.hpp"

#include "zigpc_gateway_int.h"

extern "C" {

// Shared Unify includes
#include "datastore_mock.h"

// ZigPC includes
#include "zigpc_datastore_mock.h"
#include "zigbee_host_mock.h"

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

void test_zigpc_gateway_load_address_table_info_empty(void)
{
  int64_t dev_count = 0;

  // ARRANGE
  datastore_fetch_int_ExpectAndReturn(ZIGPC_GATEWAY_NODEID_COUNT_KEY,
                                      nullptr,
                                      SL_STATUS_OK);
  datastore_fetch_int_IgnoreArg_value();
  datastore_fetch_int_ReturnThruPtr_value(&dev_count);

  // ACT
  sl_status_t status = zigpc_gateway_load_address_table_info();

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_gateway_load_address_table_info_multiple(void)
{
  int64_t dev_count                              = 2;
  zigpc_gateway_addr_entry_t mappings[dev_count] = {
    {{0xAA, 0x92, 0x60, 0x00, 0x43, 0x36, 0x32, 0x02}, 0x1234},
    {{0xBB, 0xF1, 0x93, 0xF3, 0x63, 0x06, 0x21, 0x00}, 0xABCD},
  };

  // ARRANGE
  datastore_fetch_int_ExpectAndReturn(ZIGPC_GATEWAY_NODEID_COUNT_KEY,
                                      nullptr,
                                      SL_STATUS_OK);
  datastore_fetch_int_IgnoreArg_value();
  datastore_fetch_int_ReturnThruPtr_value(&dev_count);

  datastore_fetch_arr_ExpectAndReturn(ZIGPC_GATEWAY_NODEID_LIST_KEY,
                                      nullptr,
                                      nullptr,
                                      SL_STATUS_OK);
  datastore_fetch_arr_IgnoreArg_value();
  datastore_fetch_arr_IgnoreArg_size();
  datastore_fetch_arr_ReturnMemThruPtr_value(
    reinterpret_cast<uint8_t *>(mappings),
    sizeof(zigpc_gateway_addr_entry_t) * dev_count);

  for (int64_t i = 0; i < dev_count; i++) {
    zigbeeHostAddAddressTableEntry_ExpectAndReturn(mappings[i].eui64,
                                                   mappings[i].node_id,
                                                   SL_STATUS_OK );
  }

  // ACT
  sl_status_t status = zigpc_gateway_load_address_table_info();

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_gateway_persist_address_table_info_empty(void)
{
  int64_t dev_count = 0;

  // ARRANGE
  zigpc_datastore_get_device_count_ExpectAndReturn(dev_count);

  datastore_store_int_ExpectAndReturn(ZIGPC_GATEWAY_NODEID_COUNT_KEY,
                                      dev_count,
                                      SL_STATUS_OK);

  // ACT
  sl_status_t status = zigpc_gateway_persist_address_table_info();

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_gateway_persist_address_table_info_multiple(void)
{
  int64_t dev_count = 3;

  // NOTE: eui64 is stored as the EmberAf/LE representation
  zigpc_gateway_addr_entry_t mappings[dev_count] = {
    {{0xAA, 0x92, 0x60, 0x00, 0x43, 0x36, 0x32, 0x02}, 0xAA23},
    {{0xBB, 0xF1, 0x93, 0xF3, 0x63, 0x06, 0x21, 0x00}, 0x9342},
    {{0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00}, 0x4235},
  };

  zigbee_eui64_t dev_list[dev_count] = {
    {0x02, 0x32, 0x36, 0x43, 0x00, 0x60, 0x92, 0xAA},
    {0x00, 0x21, 0x06, 0x63, 0xF3, 0x93, 0xF1, 0xBB},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07},
  };

  // ARRANGE
  zigpc_datastore_get_device_count_ExpectAndReturn(dev_count);
  for (int64_t i = 0; i < dev_count; i++) {
    // Expect calls for retrieving known device EUI64s
    zigpc_datastore_find_device_by_index_ExpectAndReturn(i, NULL, SL_STATUS_OK);
    zigpc_datastore_find_device_by_index_IgnoreArg_eui64();
    zigpc_datastore_find_device_by_index_ReturnThruPtr_eui64(&dev_list[i]);

    zigbeeHostGetAddressTableEntry_ExpectAndReturn(mappings[i].eui64,
                                                   nullptr,
                                                   SL_STATUS_OK );
    zigbeeHostGetAddressTableEntry_IgnoreArg_nodeId();

    sl_802154_short_addr_t node_id = 0;
    zigbeeHostGetAddressTableEntry_ReturnThruPtr_nodeId(&node_id);
    mappings[i].node_id = node_id;
  }

  datastore_store_arr_ExpectAndReturn(ZIGPC_GATEWAY_NODEID_LIST_KEY,
                                      reinterpret_cast<uint8_t *>(mappings),
                                      sizeof(zigpc_gateway_addr_entry_t)
                                        * dev_count,
                                      SL_STATUS_OK);

  datastore_store_int_ExpectAndReturn(ZIGPC_GATEWAY_NODEID_COUNT_KEY,
                                      dev_count,
                                      SL_STATUS_OK);

  // ACT
  sl_status_t status = zigpc_gateway_persist_address_table_info();

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

} /* extern C */
