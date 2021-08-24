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

#include "device-table_mock.h"
#include "z3gateway.h"

/**
 * @brief Setup the test suite (called once before all test_xxx functions are
 * called)
 *
 */
void suiteSetUp(void) {}

/**
 * @brief Teardown the test suite (called once after all test_xxx functions are
 * called)
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

/************************************************************
 * z3gatewayDeviceTableImportEntry tests
 ***********************************************************/

/**
 * @brief Test scenario of import API handling invalid input
 *
 */
void test_device_table_import_should_handle_invalid_argument(void)
{
  // ARRANGE
  EmberStatus null_ptr_status;
  EmberStatus over_capacity_status;
  struct z3gatewayDeviceTableStorageEntry entry = {0};

  // ACT
  null_ptr_status      = z3gatewayDeviceTableImportEntry(NULL, 0);
  over_capacity_status = z3gatewayDeviceTableImportEntry(
    &entry,
    EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_BAD_ARGUMENT, null_ptr_status);
  TEST_ASSERT_EQUAL(EMBER_INDEX_OUT_OF_RANGE, over_capacity_status);
}

/**
 * @brief Test scenario of importing empty list
 *
 */
void test_device_table_import_should_handle_cluster_list_overrun(void)
{
  // ARRANGE
  EmberStatus status;
  static const size_t cluster_id_count
    = EMBER_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE + 1;
  uint16_t cluster_id_list[cluster_id_count];
  struct z3gatewayDeviceTableStorageEntry entry_list = {
    .eui64                     = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF},
    .nodeId                    = 0x1234,
    .endpoint                  = 1,
    .deviceId                  = 0x1000,
    .clusterIdCount            = cluster_id_count,
    .clusterIdClientStartIndex = 1,
    .clusterIdList             = cluster_id_list,
  };

  /**
   * Table should be cleared once at beginning and another when buffer overrun
   * is detected.
   */

  // ACT
  status = z3gatewayDeviceTableImportEntry(&entry_list, 0);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_INDEX_OUT_OF_RANGE, status);
}

/**
 * @brief Test scenario of importing one entry
 *
 */
void test_device_table_import_should_handle_single_entry_list(void)
{
  // ARRANGE
  EmberStatus status;
  EmberAfPluginDeviceTableEntry dest_entry;
  uint16_t cluster_id_list[2]                        = {0, 1};
  struct z3gatewayDeviceTableStorageEntry entry_list = {
    .eui64                     = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF},
    .nodeId                    = 0x1234,
    .endpoint                  = 1,
    .deviceId                  = 0x1000,
    .clusterIdCount            = 2,
    .clusterIdClientStartIndex = 1,
    .clusterIdList             = cluster_id_list,
  };

  emberAfDeviceTableFindDeviceTableEntry_ExpectAndReturn(0, &dest_entry);

  // ACT
  status = z3gatewayDeviceTableImportEntry(&entry_list, 0);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_SUCCESS, status);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(entry_list.eui64, dest_entry.eui64, EUI64_SIZE);
  TEST_ASSERT_EQUAL(entry_list.nodeId, dest_entry.nodeId);
  TEST_ASSERT_EQUAL(entry_list.endpoint, dest_entry.endpoint);
  TEST_ASSERT_EQUAL(entry_list.deviceId, dest_entry.deviceId);
  TEST_ASSERT_EQUAL(entry_list.clusterIdClientStartIndex,
                    dest_entry.clusterOutStartPosition);
  TEST_ASSERT_EQUAL_HEX16_ARRAY(entry_list.clusterIdList,
                                dest_entry.clusterIds,
                                entry_list.clusterIdCount);
}

/**
 * @brief Test scenario of importing multiple entries
 *
 */
void test_device_table_import_should_handle_multi_entry_list(void)
{
  // ARRANGE
  EmberStatus status_entry[2];
  EmberAfPluginDeviceTableEntry dest_entry[2];
  uint16_t entry1_cluster_id_list[2]                   = {0, 1};
  uint16_t entry2_cluster_id_list[4]                   = {1, 3, 5, 6};
  struct z3gatewayDeviceTableStorageEntry entry_list[] = {
    {
      .eui64                     = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF},
      .nodeId                    = 0x1234,
      .endpoint                  = 1,
      .deviceId                  = 0x1000,
      .clusterIdCount            = 2,
      .clusterIdClientStartIndex = 1,
      .clusterIdList             = entry1_cluster_id_list,
    },
    {
      .eui64                     = {0x0, 0xE, 0xA, 0xD, 0x0, 0xE, 0xE, 0x0},
      .nodeId                    = 0x4321,
      .endpoint                  = 2,
      .deviceId                  = 0x1004,
      .clusterIdCount            = 4,
      .clusterIdClientStartIndex = 0,
      .clusterIdList             = entry2_cluster_id_list,
    },
  };

  for (uint16_t i = 0; i < 2; i++) {
    emberAfDeviceTableFindDeviceTableEntry_ExpectAndReturn(i, &dest_entry[i]);

    // ACT
    status_entry[i] = z3gatewayDeviceTableImportEntry(&entry_list[i], i);

    // ASSERT
    TEST_ASSERT_EQUAL(EMBER_SUCCESS, status_entry[i]);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(entry_list[i].eui64,
                                 dest_entry[i].eui64,
                                 EUI64_SIZE);
    TEST_ASSERT_EQUAL(entry_list[i].nodeId, dest_entry[i].nodeId);
    TEST_ASSERT_EQUAL(entry_list[i].endpoint, dest_entry[i].endpoint);
    TEST_ASSERT_EQUAL(entry_list[i].deviceId, dest_entry[i].deviceId);
    TEST_ASSERT_EQUAL(entry_list[i].clusterIdClientStartIndex,
                      dest_entry[i].clusterOutStartPosition);
    TEST_ASSERT_EQUAL_HEX16_ARRAY(entry_list[i].clusterIdList,
                                  dest_entry[i].clusterIds,
                                  entry_list[i].clusterIdCount);
  }
}

/************************************************************
 * z3gatewayDeviceTableExport tests
 ***********************************************************/

/**
 * @brief Test scenario of export API handling invalid input
 *
 */

void test_device_table_export_should_handle_invalid_argument(void)
{
  // ARRANGE
  EmberStatus null_ptr_status;
  EmberStatus invalid_index_status;
  struct z3gatewayDeviceTableStorageEntry entry;

  // ACT
  null_ptr_status      = z3gatewayDeviceTableExportEntry(NULL, 0);
  invalid_index_status = z3gatewayDeviceTableExportEntry(
    &entry,
    EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_BAD_ARGUMENT, null_ptr_status);
  TEST_ASSERT_EQUAL(EMBER_INDEX_OUT_OF_RANGE, invalid_index_status);
}

/**
 * @brief Test scenario of exporting list with large cluster list
 *
 */
void test_device_table_export_should_handle_cluster_list_overrun(void)
{
  // ARRANGE
  EmberStatus status;
  uint16_t cluster_list[2];
  struct z3gatewayDeviceTableStorageEntry entry = {
    .clusterIdList  = cluster_list,
    .clusterIdCount = 2,
  };
  EmberAfPluginDeviceTableEntry table_entry = {
    .nodeId     = 0x1234,
    .clusterIds = {0, 1, 2, 3, 4},
  };

  emberAfDeviceTableFindDeviceTableEntry_ExpectAndReturn(0, &table_entry);
  emberAfDeviceTableFindDeviceTableEntry_ExpectAndReturn(0, &table_entry);

  // ACT
  status = z3gatewayDeviceTableExportEntry(&entry, 0);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_INDEX_OUT_OF_RANGE, status);
}

/**
 * @brief Test scenario of exporting empty list
 *
 */
void test_device_table_export_should_handle_empty_list(void)
{
  // ARRANGE
  EmberStatus status;
  uint16_t cluster_list[5];
  struct z3gatewayDeviceTableStorageEntry entry = {
    .clusterIdList = cluster_list,
  };
  EmberAfPluginDeviceTableEntry table_entries[] = {
    {.nodeId = EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID},
  };

  emberAfDeviceTableFindDeviceTableEntry_ExpectAndReturn(0, &table_entries[0]);

  // ACT
  status = z3gatewayDeviceTableExportEntry(&entry, 0);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_NOT_FOUND, status);
}

/**
 * @brief Test scenario of exporting list with one item
 *
 */
void test_device_table_export_should_handle_non_empty_list(void)
{
  // ARRANGE
  EmberStatus status;
  uint16_t export_cluster_list[EMBER_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE];
  struct z3gatewayDeviceTableStorageEntry export_entry = {
    .clusterIdList  = export_cluster_list,
    .clusterIdCount = EMBER_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE,
  };
  EmberAfPluginDeviceTableEntry table_entries[11] = {0};

  // Setup 10 table entries to be exported
  for (size_t i = 0; i < 10; i++) {
    for (size_t j = 0; j < EMBER_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE; j++) {
      table_entries[i].clusterIds[j] = j;
    }
    table_entries[i].nodeId   = i + 1;
    table_entries[i].endpoint = i + 1;
    table_entries[i].deviceId = i + 1;
    table_entries[i].eui64[0] = i + 1;
    emberAfDeviceTableFindDeviceTableEntry_ExpectAndReturn(i,
                                                           &table_entries[i]);
    emberAfDeviceTableFindDeviceTableEntry_ExpectAndReturn(i,
                                                           &table_entries[i]);
  }
  // 11th entry should have the NULL NODE ID
  table_entries[10].nodeId = EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID;
  emberAfDeviceTableFindDeviceTableEntry_ExpectAndReturn(10,
                                                         &table_entries[10]);

  for (size_t i = 0; i < 10; i++) {
    // ACT
    status = z3gatewayDeviceTableExportEntry(&export_entry, i);

    // ASSERT
    TEST_ASSERT_EQUAL(EMBER_SUCCESS, status);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(table_entries[i].eui64,
                                 export_entry.eui64,
                                 EUI64_SIZE);
    TEST_ASSERT_EQUAL(table_entries[i].nodeId, export_entry.nodeId);
    TEST_ASSERT_EQUAL(table_entries[i].endpoint, export_entry.endpoint);
    TEST_ASSERT_EQUAL(table_entries[i].deviceId, export_entry.deviceId);
    TEST_ASSERT_EQUAL(table_entries[i].clusterOutStartPosition,
                      export_entry.clusterIdClientStartIndex);
    TEST_ASSERT_EQUAL_HEX16_ARRAY(table_entries[i].clusterIds,
                                  export_entry.clusterIdList,
                                  export_entry.clusterIdCount);
  }

  // ACT
  status = z3gatewayDeviceTableExportEntry(&export_entry, 10);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_NOT_FOUND, status);
}

/**
 * @brief Test scenario of exporting full list
 *
 */
void test_device_table_export_should_handle_full_list(void)
{
  // ARRANGE
  EmberStatus status;
  uint16_t export_cluster_list[EMBER_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE];
  struct z3gatewayDeviceTableStorageEntry export_entry = {
    .clusterIdList  = export_cluster_list,
    .clusterIdCount = EMBER_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE,
  };

  // Setup 10 table entries to be exported
  EmberAfPluginDeviceTableEntry
    table_entries[EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE]
    = {0};

  for (uint16_t i = 0; i < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE;
       i++) {
    // Set nodeId to not be invalid (0xFFFF)
    table_entries[i].nodeId = i;

    // set each Cluster ID to not be invalid (0xFFFF)
    for (size_t j = 0; j < EMBER_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE; j++) {
      table_entries[i].clusterIds[j]
        = (j % EMBER_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE);
    }
    table_entries[i].nodeId   = i + 1;
    table_entries[i].endpoint = i + 1;
    table_entries[i].deviceId = i + 1;
    table_entries[i].eui64[0] = i + 1;
    emberAfDeviceTableFindDeviceTableEntry_ExpectAndReturn(i,
                                                           &table_entries[i]);
    emberAfDeviceTableFindDeviceTableEntry_ExpectAndReturn(i,
                                                           &table_entries[i]);
  }

  for (uint16_t i = 0; i < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE;
       i++) {
    // ACT
    status = z3gatewayDeviceTableExportEntry(&export_entry, i);

    // ASSERT
    TEST_ASSERT_EQUAL(EMBER_SUCCESS, status);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(table_entries[i].eui64,
                                 export_entry.eui64,
                                 EUI64_SIZE);
    TEST_ASSERT_EQUAL(table_entries[i].nodeId, export_entry.nodeId);
    TEST_ASSERT_EQUAL(table_entries[i].endpoint, export_entry.endpoint);
    TEST_ASSERT_EQUAL(table_entries[i].deviceId, export_entry.deviceId);
    TEST_ASSERT_EQUAL(table_entries[i].clusterOutStartPosition,
                      export_entry.clusterIdClientStartIndex);
    TEST_ASSERT_EQUAL_HEX16_ARRAY(table_entries[i].clusterIds,
                                  export_entry.clusterIdList,
                                  EMBER_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE);
  }
}
