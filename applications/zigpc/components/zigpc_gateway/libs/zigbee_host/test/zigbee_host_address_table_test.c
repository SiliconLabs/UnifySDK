/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include <unity.h>

// Zigbee stack includes
#define SIGNED_ENUM
#include "af_mock.h"
#include "address-table_mock.h"

#include "zigbee_host.h"

/**
 * @brief  Setup the test suite
 * (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp(void) {}

/**
 * @brief  Teardown the test suite
 * (called once after all test_xxx functions are called)
 *
 * @param num_failures
 * @return int
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/**
 * @brief Setup before each test.
 *
 */
void setUp(void) {}

/**
 * @brief Teardown after each test.
 *
 */
void tearDown(void) {}

void test_address_table_add_entry_sanity(void)
{
  sl_802154_long_addr_t eui64   = "\x12\x03\x39\xFA\x0B\x3A\x99\x7E";
  sl_802154_short_addr_t nodeId = 0xF930;

  // ARRANGE
  sl_zigbee_af_add_address_table_entry_ExpectAndReturn(eui64, nodeId, SL_STATUS_OK );

  // ACT
  sl_status_t status = zigbeeHostAddAddressTableEntry(eui64, nodeId);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK , status);
}

void test_address_table_add_entry_invalid_eui64(void)
{
  sl_802154_short_addr_t nodeId = 0xF930;

  // ARRANGE

  // ACT
  sl_status_t status = zigbeeHostAddAddressTableEntry(NULL, nodeId);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
}

void test_address_table_get_entry_sanity(void)
{
  sl_802154_long_addr_t eui64           = "\x12\x03\x1B\xFA\x0B\x3A\x99\x7E";
  sl_802154_short_addr_t expectedNodeId = 0x231D;

  // ARRANGE
  sl_zigbee_af_lookup_address_table_entry_by_eui64_ExpectAndReturn(eui64, 0);
  sl_zigbee_af_address_table_lookup_node_id_by_index_ExpectAndReturn(0,
                                                               expectedNodeId);

  // ACT
  sl_802154_short_addr_t nodeId;
  sl_status_t status = zigbeeHostGetAddressTableEntry(eui64, &nodeId);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK , status);
  TEST_ASSERT_EQUAL_HEX16(expectedNodeId, nodeId);
}

void test_address_table_get_entry_invalid_eui64(void)
{
  // ARRANGE

  // ACT
  sl_status_t status = zigbeeHostGetAddressTableEntry(NULL, NULL);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
}
