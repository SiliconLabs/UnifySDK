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

#include "datastore_mock.h"
#include "z3gateway_mock.h"
#include "zigpc_common_zigbee_mock.h"

static const char *DS_DEVICE_TABLE_SIZE_KEY
  = ZIGPC_GATEWAY_STORE_DEVICE_TABLE_SIZE_KEY;
static const char *DS_DEVICE_TABLE_ARR_KEY
  = ZIGPC_GATEWAY_STORE_DEVICE_TABLE_ARR_KEY;
static const char *DS_DEVICE_TABLE_CLUSTER_ARR_KEY
  = ZIGPC_GATEWAY_STORE_DEVICE_TABLE_CLUSTER_ARR_KEY;

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

/**
 * @brief Test scenario of entries from datastore not found
 *
 */
void test_zigpc_gateway_handles_entries_not_found_from_datastore(void)
{
  // ARRANGE
  sl_status_t status;

  // Expect reading table size from datastore
  datastore_fetch_arr_ExpectAndReturn(DS_DEVICE_TABLE_SIZE_KEY,
                                      NULL,
                                      NULL,
                                      SL_STATUS_NOT_FOUND);
  datastore_fetch_arr_IgnoreArg_value();
  datastore_fetch_arr_IgnoreArg_size();

  // ACT
  status = zigpc_gateway_load_from_datastore();

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
}

/**
 * @brief Test scenario of loading empty entries from datastore
 *
 */
void test_zigpc_gateway_handles_zero_entries_from_datastore(void)
{
  // ARRANGE
  sl_status_t status;
  uint16_t ds_dev_table_count          = 0;
  unsigned int ds_dev_table_count_size = sizeof(uint16_t);

  // Expect reading table size from datastore
  datastore_fetch_arr_ExpectAndReturn(DS_DEVICE_TABLE_SIZE_KEY,
                                      NULL,
                                      NULL,
                                      SL_STATUS_OK);
  datastore_fetch_arr_IgnoreArg_value();
  datastore_fetch_arr_IgnoreArg_size();
  datastore_fetch_arr_ReturnMemThruPtr_value(
    reinterpret_cast<uint8_t *>(&ds_dev_table_count),
    ds_dev_table_count_size);
  datastore_fetch_arr_ReturnThruPtr_size(&ds_dev_table_count_size);

  // ACT
  status = zigpc_gateway_load_from_datastore();

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
}

/**
 * @brief Test happy path scenario of loading information from datastore
 *
 */
void test_zigpc_gateway_loads_from_multiple_entries_from_datastore(void)
{
  // ARRANGE
  sl_status_t status;
  std::string expected_key;
  // Entry counts/sizes
  uint16_t ds_dev_table_count         = 13;
  uint16_t ds_dev_table_cluster_count = 32;
  // Entry storage
  struct z3gatewayDeviceTableStorageEntry ds_dev_table_list[ds_dev_table_count]
    = {0};
  uint16_t ds_dev_table_cluster_lists[ds_dev_table_count]
                                     [ds_dev_table_cluster_count];

  // Datastore sizes
  unsigned int ds_dev_table_count_size = sizeof(uint16_t);
  unsigned int ds_dev_table_entry_size
    = sizeof(struct z3gatewayDeviceTableStorageEntry);
  unsigned int ds_dev_table_cluster_size
    = sizeof(uint16_t) * ds_dev_table_cluster_count;

  // Datastore key character array pointers
  char *ds_entry_keys[ds_dev_table_count];
  char *ds_entry_cluster_keys[ds_dev_table_count];

  // Expect reading table size from datastore
  datastore_fetch_arr_ExpectAndReturn(DS_DEVICE_TABLE_SIZE_KEY,
                                      NULL,
                                      NULL,
                                      SL_STATUS_OK);
  datastore_fetch_arr_IgnoreArg_value();
  datastore_fetch_arr_IgnoreArg_size();
  datastore_fetch_arr_ReturnMemThruPtr_value(
    reinterpret_cast<uint8_t *>(&ds_dev_table_count),
    ds_dev_table_count_size);
  datastore_fetch_arr_ReturnThruPtr_size(&ds_dev_table_count_size);

  z3gatewayDeviceTableClear_ExpectAndReturn(SL_STATUS_OK);

  for (uint16_t i = 0; i < ds_dev_table_count; i++) {
    ds_entry_keys[i]         = nullptr;
    ds_entry_cluster_keys[i] = nullptr;

    for (size_t j = 0; j < ds_dev_table_cluster_count; j++) {
      ds_dev_table_cluster_lists[i][j] = j;
    }

    ds_dev_table_list[i].clusterIdList  = ds_dev_table_cluster_lists[i];
    ds_dev_table_list[i].clusterIdCount = ds_dev_table_cluster_count;
    ds_dev_table_list[i].clusterIdClientStartIndex = i;
    ds_dev_table_list[i].eui64[0]                  = i;
    ds_dev_table_list[i].nodeId                    = i;
    ds_dev_table_list[i].endpoint                  = i;
    ds_dev_table_list[i].deviceId                  = i;

    // Expect reading table entry[i] from datastore
    expected_key = DS_DEVICE_TABLE_ARR_KEY;
    expected_key += "_" + std::to_string(i);
    ds_entry_keys[i] = new (std::nothrow) char[expected_key.length() + 1];
    strcpy(ds_entry_keys[i], expected_key.c_str());
    datastore_fetch_arr_ExpectAndReturn(ds_entry_keys[i],
                                        NULL,
                                        NULL,
                                        SL_STATUS_OK);
    datastore_fetch_arr_IgnoreArg_value();
    datastore_fetch_arr_IgnoreArg_size();
    datastore_fetch_arr_ReturnMemThruPtr_value(
      reinterpret_cast<uint8_t *>(&ds_dev_table_list[i]),
      ds_dev_table_entry_size);
    datastore_fetch_arr_ReturnThruPtr_size(&ds_dev_table_entry_size);

    // Expect reading table entry[i] cluster list from datastore
    expected_key = DS_DEVICE_TABLE_CLUSTER_ARR_KEY;
    expected_key += "_" + std::to_string(i);
    ds_entry_cluster_keys[i]
      = new (std::nothrow) char[expected_key.length() + 1];
    strcpy(ds_entry_cluster_keys[i], expected_key.c_str());
    datastore_fetch_arr_ExpectAndReturn(ds_entry_cluster_keys[i],
                                        NULL,
                                        NULL,
                                        SL_STATUS_OK);
    datastore_fetch_arr_IgnoreArg_value();
    datastore_fetch_arr_IgnoreArg_size();
    datastore_fetch_arr_ReturnMemThruPtr_value(
      reinterpret_cast<uint8_t *>(&ds_dev_table_cluster_lists[i]),
      ds_dev_table_cluster_size);
    datastore_fetch_arr_ReturnThruPtr_size(&ds_dev_table_cluster_size);

    z3gatewayDeviceTableImportEntry_ExpectAndReturn(&ds_dev_table_list[i],
                                                    i,
                                                    SL_STATUS_OK);
  }

  // ACT
  status = zigpc_gateway_load_from_datastore();

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);

  for (uint16_t i = 0; i < ds_dev_table_count; i++) {
    if (ds_entry_keys[i] != nullptr) {
      delete[] ds_entry_keys[i];
    }
    if (ds_entry_cluster_keys[i] != nullptr) {
      delete[] ds_entry_cluster_keys[i];
    }
  }
}

/**
 * @brief Test scenario of storing information zero entries to datastore
 *
 */
void test_zigpc_gateway_stores_zero_entries_to_datastore()
{
  // ARRANGE
  sl_status_t status          = SL_STATUS_FAIL;
  uint16_t ds_dev_table_count = 0;

  z3gatewayDeviceTableCount_ExpectAndReturn(ds_dev_table_count);

  datastore_store_arr_ExpectAndReturn(DS_DEVICE_TABLE_SIZE_KEY,
                                      NULL,
                                      sizeof(uint16_t),
                                      SL_STATUS_OK);
  datastore_store_arr_IgnoreArg_value();

  // ACT
  status = zigpc_gateway_save_to_datastore();

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
}

/**
 * @brief Test happy path scenario of storing information to datastore
 *
 */
void test_zigpc_gateway_stores_multiple_entries_to_datastore()
{
  // ARRANGE
  sl_status_t status;
  std::string expected_key;

  // Entry counts/sizes
  uint16_t ds_dev_table_count         = 10;
  uint16_t ds_dev_table_cluster_count = 45;
  // Entry storage
  struct z3gatewayDeviceTableStorageEntry ds_dev_table_list[ds_dev_table_count]
    = {0};
  uint16_t ds_dev_table_cluster_lists[ds_dev_table_count]
                                     [ds_dev_table_cluster_count];

  // Datastore key character array pointers
  char *ds_entry_keys[ds_dev_table_count];
  char *ds_entry_cluster_keys[ds_dev_table_count];

  z3gatewayDeviceTableCount_ExpectAndReturn(ds_dev_table_count);

  for (uint16_t i = 0; i < ds_dev_table_count; i++) {
    ds_entry_keys[i]         = nullptr;
    ds_entry_cluster_keys[i] = nullptr;
    // Setup device table entries to not have invalid clusterIDs
    for (size_t j = 0; j < ds_dev_table_cluster_count; j++) {
      ds_dev_table_cluster_lists[i][j] = j;
    }
    ds_dev_table_list[i].clusterIdList  = ds_dev_table_cluster_lists[i];
    ds_dev_table_list[i].clusterIdCount = ds_dev_table_cluster_count;

    z3gatewayDeviceTableEntryClusterListCount_ExpectAndReturn(
      i,
      ds_dev_table_cluster_count);

    z3gatewayDeviceTableExportEntry_ExpectAndReturn(NULL, i, SL_STATUS_OK);
    z3gatewayDeviceTableExportEntry_IgnoreArg_entry();
    z3gatewayDeviceTableExportEntry_ReturnMemThruPtr_entry(
      &ds_dev_table_list[i],
      sizeof(struct z3gatewayDeviceTableStorageEntry));

    // Expect reading table entry[i] cluster list from datastore
    expected_key = DS_DEVICE_TABLE_CLUSTER_ARR_KEY;
    expected_key += "_" + std::to_string(i);
    ds_entry_cluster_keys[i]
      = new (std::nothrow) char[expected_key.length() + 1];
    strcpy(ds_entry_cluster_keys[i], expected_key.c_str());
    datastore_store_arr_ExpectAndReturn(ds_entry_cluster_keys[i],
                                        NULL,
                                        sizeof(uint16_t)
                                          * ds_dev_table_cluster_count,
                                        SL_STATUS_OK);
    datastore_store_arr_IgnoreArg_value();

    expected_key = DS_DEVICE_TABLE_ARR_KEY;
    expected_key += "_" + std::to_string(i);
    ds_entry_keys[i] = new (std::nothrow) char[expected_key.length() + 1];
    strcpy(ds_entry_keys[i], expected_key.c_str());
    datastore_store_arr_ExpectWithArrayAndReturn(
      ds_entry_keys[i],
      expected_key.length(),
      reinterpret_cast<uint8_t *>(&ds_dev_table_list[i]),
      sizeof(struct z3gatewayDeviceTableStorageEntry),
      sizeof(struct z3gatewayDeviceTableStorageEntry),
      SL_STATUS_OK);
  }

  datastore_store_arr_ExpectAndReturn(DS_DEVICE_TABLE_SIZE_KEY,
                                      NULL,
                                      sizeof(uint16_t),
                                      SL_STATUS_OK);
  datastore_store_arr_IgnoreArg_value();

  // ACT
  status = zigpc_gateway_save_to_datastore();

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);

  for (uint16_t i = 0; i < ds_dev_table_count; i++) {
    if (ds_entry_keys[i] != nullptr) {
      delete[] ds_entry_keys[i];
    }
    if (ds_entry_cluster_keys[i] != nullptr) {
      delete[] ds_entry_cluster_keys[i];
    }
  }
}

} /* extern C */
