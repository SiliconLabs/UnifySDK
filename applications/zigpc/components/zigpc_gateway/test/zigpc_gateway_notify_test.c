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

#include "unity.h"

#include "zigpc_common_observable_mock.h"
#include "zigpc_common_zigbee_mock.h"

#include "zigpc_gateway_notify.h"
#include "zigpc_gateway_int.h"

#include "z3gateway_callbacks.h"

/* Prototypes for notify functions under test */
void zigpc_gateway_hdl_on_network_initialized(
  const EmberEUI64 eui64_le, const EmberNetworkParameters *network);

void zigpc_gateway_hdl_on_node_add_start(const EmberEUI64 eui64_le);

void zigpc_gateway_hdl_on_node_add_complete(const EmberEUI64 eui64_le);

void zigpc_gateway_hdl_on_node_discovered(const EmberEUI64 eui64_le,
                                          uint8_t endpointCount);
void zigpc_gateway_hdl_on_node_endpoint_discovered(
  const EmberEUI64 eui64_le, const struct z3gatewayEndpointInfo *endpointInfo);

void zigpc_gateway_hdl_on_node_removed(const EmberEUI64 eui64_le);

/**
 * @brief Setup the test suite (called once before all test_xxx functions are
 * called)
 *
 */
void suiteSetUp() {}

/**
 * @brief Teardown the test suite (called once after all test_xxx functions are
 * called)
 *
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp() {}

void tearDown() {}

/**
 * @brief Test to check delegating reset to common observer
 *
 */
void test_zigpc_gateway_notify_reset_should_call_common_observer(void)
{
  // ARRANGE
  zigpc_observable_clear_Expect(&zigpc_gateway_observable);
  zigpc_observable_init_ExpectAndReturn(&zigpc_gateway_observable,
                                        NULL,
                                        ZIGPC_GATEWAY_NOTIFY_MAX,
                                        SL_STATUS_OK);
  zigpc_observable_init_IgnoreArg_event_list();

  // ACT
  sl_status_t result = zigpc_gateway_reset_observers();

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, result);
}

/**
 * @brief Test to check failing to reset common observer
 *
 */
void test_zigpc_gateway_notify_reset_should_error_on_failed_common_observer(
  void)
{
  // ARRANGE
  zigpc_observable_clear_Expect(&zigpc_gateway_observable);
  zigpc_observable_init_ExpectAndReturn(&zigpc_gateway_observable,
                                        NULL,
                                        ZIGPC_GATEWAY_NOTIFY_MAX,
                                        SL_STATUS_FAIL);
  zigpc_observable_init_IgnoreArg_event_list();

  // ACT
  sl_status_t result = zigpc_gateway_reset_observers();

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, result);
}

/**
 * @brief Test to check delegating registering to common observer
 *
 */
void test_zigpc_gateway_notify_register_should_call_common_observer(void)
{
  // ARRANGE
  sl_status_t result;

  zigpc_observable_register_ExpectAndReturn(&zigpc_gateway_observable,
                                            2,
                                            NULL,
                                            SL_STATUS_OK);

  // ACT
  result = zigpc_gateway_register_observer(2, NULL);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, result);
}

/**
 * @brief Test to check delegating unregistering to common observer
 *
 */
void test_zigpc_gateway_notify_unregister_should_call_common_observer(void)
{
  // ARRANGE
  sl_status_t result;

  zigpc_observable_unregister_ExpectAndReturn(&zigpc_gateway_observable,
                                              1,
                                              NULL,
                                              SL_STATUS_OK);

  // ACT
  result = zigpc_gateway_unregister_observer(1, NULL);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, result);
}

/**
 * @brief Test to check sending network_init event via common observer notify
 *
 */
void test_zigpc_gateway_network_init_should_call_common_notify(void)
{
  // ARRANGE
  EmberEUI64 test_eui64_le = {0x1, 0x2, 0x3, 0x0};
  EmberNetworkParameters test_network_params
    = {.panId = 0xF8, .extendedPanId = {0xF8, 0}, .radioChannel = 0xAB};

  zigbee_eui64_copy_switch_endian_Expect(NULL, test_eui64_le);
  zigbee_eui64_copy_switch_endian_IgnoreArg_dst();

  zigpc_observable_notify_ExpectAndReturn(&zigpc_gateway_observable,
                                          ZIGPC_GATEWAY_NOTIFY_NETWORK_INIT,
                                          NULL,
                                          SL_STATUS_OK);
  zigpc_observable_notify_IgnoreArg_data();

  // ACT
  zigpc_gateway_hdl_on_network_initialized(test_eui64_le, &test_network_params);

  // ASSERT (Handled by CMock)
}

/**
 * @brief Test to check sending node_add_start event via common observer notify
 *
 */
void test_zigpc_gateway_node_start_should_call_common_notify(void)
{
  // ARRANGE
  EmberEUI64 test_eui64_le          = {0xA, 0xB, 0xC, 0xD, 0x0};
  EmberEUI64 test_eui64_be          = {0x0, 0x0, 0x0, 0x0, 0xD, 0xC, 0xB, 0xA};
  zigbee_eui64_uint_t eui64_uint_be = 0x0A0B0C0D00000000;

  zigbee_eui64_copy_switch_endian_Expect(NULL, test_eui64_le);
  zigbee_eui64_copy_switch_endian_IgnoreArg_dst();
  zigbee_eui64_copy_switch_endian_ReturnThruPtr_dst(test_eui64_be);

  zigbee_eui64_to_uint_ExpectAndReturn(test_eui64_be, eui64_uint_be);

  zigpc_observable_notify_ExpectAndReturn(&zigpc_gateway_observable,
                                          ZIGPC_GATEWAY_NOTIFY_NODE_ADD_START,
                                          NULL,
                                          SL_STATUS_OK);
  zigpc_observable_notify_IgnoreArg_data();

  // ACT
  zigpc_gateway_hdl_on_node_add_start(test_eui64_le);

  // ASSERT Handled by CMock
}

/**
 * @brief Test to check sending node_add_complete event via common observer notify
 *
 */
void test_zigpc_gateway_node_complete_should_call_common_notify(void)
{
  // ARRANGE
  EmberEUI64 test_eui64_le          = {0xA, 0xB, 0xC, 0xD, 0x0};
  EmberEUI64 test_eui64_be          = {0x0, 0x0, 0x0, 0x0, 0xD, 0xC, 0xB, 0xA};
  zigbee_eui64_uint_t eui64_uint_be = 0x0A0B0C0D00000000;

  zigbee_eui64_copy_switch_endian_Expect(NULL, test_eui64_le);
  zigbee_eui64_copy_switch_endian_IgnoreArg_dst();
  zigbee_eui64_copy_switch_endian_ReturnThruPtr_dst(test_eui64_be);

  zigbee_eui64_to_uint_ExpectAndReturn(test_eui64_be, eui64_uint_be);

  zigpc_observable_notify_ExpectAndReturn(
    &zigpc_gateway_observable,
    ZIGPC_GATEWAY_NOTIFY_NODE_ADD_COMPLETE,
    NULL,
    SL_STATUS_OK);
  zigpc_observable_notify_IgnoreArg_data();

  // ACT
  zigpc_gateway_hdl_on_node_add_complete(test_eui64_le);

  // ASSERT (Handled by CMock)
}

/**
 * @brief Test to check sending device_discovered event via common observer notify
 *
 */
void test_zigpc_gateway_device_discovered_should_call_common_notify(void)
{
  // ARRANGE
  EmberEUI64 test_eui64_le          = {0xA, 0xB, 0xC, 0xD, 0x0};
  EmberEUI64 test_eui64_be          = {0x0, 0x0, 0x0, 0x0, 0xD, 0xC, 0xB, 0xA};
  zigbee_eui64_uint_t eui64_uint_be = 0x0A0B0C0D00000000;
  uint8_t epCount                   = 7;
  zigpc_gateway_on_node_discovered_t test_notify_data
    = {.endpoint_count = epCount};

  // Switch Endian to compare result
  for (int i = 0, length = sizeof(zigbee_eui64_t); i < length; i++) {
    test_notify_data.eui64[i] = test_eui64_le[length - i - 1];
  }

  zigbee_eui64_copy_switch_endian_Expect(NULL, test_eui64_le);
  zigbee_eui64_copy_switch_endian_IgnoreArg_dst();
  zigbee_eui64_copy_switch_endian_ReturnThruPtr_dst(test_eui64_be);

  zigbee_eui64_to_uint_ExpectAndReturn(test_eui64_be, eui64_uint_be);

  zigpc_observable_notify_ExpectAndReturn(&zigpc_gateway_observable,
                                          ZIGPC_GATEWAY_NOTIFY_NODE_DISCOVERED,
                                          &test_notify_data,
                                          SL_STATUS_OK);

  // ACT
  zigpc_gateway_hdl_on_node_discovered(test_eui64_le, epCount);

  // ASSERT (Handled by CMock)
}

/**
 * @brief Test to check sending device_endpoint_discovered event via common observer notify
 *
 */
void test_zigpc_gateway_endpoint_discovered_should_call_common_notify(void)
{
  // ARRANGE
  EmberEUI64 test_eui64_le          = {0xA, 0xB, 0xC, 0xD, 0x0};
  EmberEUI64 test_eui64_be          = {0x0, 0x0, 0x0, 0x0, 0xD, 0xC, 0xB, 0xA};
  zigbee_eui64_uint_t eui64_uint_be = 0x0A0B0C0D00000000;
  uint16_t epClusterList[]          = {0x6, 0x4, 0x1};
  struct z3gatewayEndpointInfo test_epinfo = {
    .endpoint           = 123,
    .deviceId           = 0x456,
    .serverClusterCount = 3,
    .serverClusterList  = epClusterList,
  };

  zigbee_eui64_copy_switch_endian_Expect(NULL, test_eui64_le);
  zigbee_eui64_copy_switch_endian_IgnoreArg_dst();
  zigbee_eui64_copy_switch_endian_ReturnThruPtr_dst(test_eui64_be);

  zigbee_eui64_to_uint_ExpectAndReturn(test_eui64_be, eui64_uint_be);

  zigpc_observable_notify_ExpectAndReturn(
    &zigpc_gateway_observable,
    ZIGPC_GATEWAY_NOTIFY_NODE_ENDPOINT_DISCOVERED,
    NULL,
    SL_STATUS_OK);
  zigpc_observable_notify_IgnoreArg_data();

  // ACT
  zigpc_gateway_hdl_on_node_endpoint_discovered(test_eui64_le, &test_epinfo);

  // ASSERT (Handled by CMock)
}

/**
 * @brief Test to check sending node_removed event via common observer notify
 *
 */
void test_zigpc_gateway_node_removed_should_call_common_notify(void)
{
  // ARRANGE
  EmberEUI64 test_eui64_le          = {0xA, 0xB, 0xC, 0xD, 0x0};
  EmberEUI64 test_eui64_be          = {0x0, 0x0, 0x0, 0x0, 0xD, 0xC, 0xB, 0xA};
  zigbee_eui64_uint_t eui64_uint_be = 0x0A0B0C0D00000000;

  zigbee_eui64_copy_switch_endian_Expect(NULL, test_eui64_le);
  zigbee_eui64_copy_switch_endian_IgnoreArg_dst();
  zigbee_eui64_copy_switch_endian_ReturnThruPtr_dst(test_eui64_be);

  zigbee_eui64_to_uint_ExpectAndReturn(test_eui64_be, eui64_uint_be);

  zigpc_observable_notify_ExpectAndReturn(&zigpc_gateway_observable,
                                          ZIGPC_GATEWAY_NOTIFY_NODE_REMOVED,
                                          NULL,
                                          SL_STATUS_OK);
  zigpc_observable_notify_IgnoreArg_data();

  // ACT
  zigpc_gateway_hdl_on_node_removed(test_eui64_le);

  // ASSERT (Handled by CMock)
}
