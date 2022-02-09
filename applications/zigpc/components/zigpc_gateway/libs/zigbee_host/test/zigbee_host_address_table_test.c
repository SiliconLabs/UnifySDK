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
  EmberEUI64 eui64   = "\x12\x03\x39\xFA\x0B\x3A\x99\x7E";
  EmberNodeId nodeId = 0xF930;

  // ARRANGE
  emberAfAddAddressTableEntry_ExpectAndReturn(eui64, nodeId, EMBER_SUCCESS);

  // ACT
  EmberStatus status = zigbeeHostAddAddressTableEntry(eui64, nodeId);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(EMBER_SUCCESS, status);
}

void test_address_table_add_entry_invalid_eui64(void)
{
  EmberNodeId nodeId = 0xF930;

  // ARRANGE

  // ACT
  EmberStatus status = zigbeeHostAddAddressTableEntry(NULL, nodeId);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(EMBER_BAD_ARGUMENT, status);
}

void test_address_table_get_entry_sanity(void)
{
  EmberEUI64 eui64           = "\x12\x03\x1B\xFA\x0B\x3A\x99\x7E";
  EmberNodeId expectedNodeId = 0x231D;

  // ARRANGE
  emberAfLookupAddressTableEntryByEui64_ExpectAndReturn(eui64, 0);
  emberAfPluginAddressTableLookupNodeIdByIndex_ExpectAndReturn(0,
                                                               expectedNodeId);

  // ACT
  EmberNodeId nodeId;
  EmberStatus status = zigbeeHostGetAddressTableEntry(eui64, &nodeId);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(EMBER_SUCCESS, status);
  TEST_ASSERT_EQUAL_HEX16(expectedNodeId, nodeId);
}

void test_address_table_get_entry_invalid_eui64(void)
{
  // ARRANGE

  // ACT
  EmberStatus status = zigbeeHostGetAddressTableEntry(NULL, NULL);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(EMBER_BAD_ARGUMENT, status);
}
