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

#include "unity.h"
#include "string.h"

#include "zigpc_discovery_mock.h"
#include "zigpc_gateway_notify_mock.h"
#include "zigpc_net_mgmt_process_send_mock.h"

#include "zigpc_net_mgmt_gateway.h"

/**
 * Prototypes of functions under test
 */

void zigpc_net_mgmt_callback_on_network_initialized(void *event_data);
void zigpc_net_mgmt_callback_on_node_add_complete(void *event_data);
void zigpc_net_mgmt_on_discovery_status(
  zigbee_eui64_uint_t eui64, zigpc_discovery_status_t discovery_status);
void zigpc_net_mgmt_callback_on_node_removed(void *event_data);

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

void setUp(void) {}

void tearDown(void) {}

void test_gateway_registrations_should_succeed(void)
{
  // ARRANGE
  enum zigpc_gateway_notify_event expected_events[] = {
    ZIGPC_GATEWAY_NOTIFY_NETWORK_INIT,
    ZIGPC_GATEWAY_NOTIFY_NODE_ADD_START,
    ZIGPC_GATEWAY_NOTIFY_NODE_ADD_COMPLETE,
    ZIGPC_GATEWAY_NOTIFY_NODE_REMOVED,
  };

  for (int i = 0, length = sizeof(expected_events) / sizeof(expected_events[0]);
       i < length;
       i++) {
    zigpc_gateway_register_observer_ExpectAndReturn(expected_events[i],
                                                    NULL,
                                                    SL_STATUS_OK);
    zigpc_gateway_register_observer_IgnoreArg_callback();
  }
  zigpc_discovery_add_listener_ExpectAndReturn(
    zigpc_net_mgmt_on_discovery_status,
    SL_STATUS_OK);

  // ACT
  sl_status_t test_status = zigpc_net_mgmt_gateway_init_observer();

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}

void test_gateway_network_init_callback_should_call_process_send(void)
{
  // ARRANGE
  struct zigpc_gateway_on_network_init callback_data;

  // ACT
  zigpc_net_mgmt_callback_on_network_initialized(&callback_data);

  // ASSERT (Handled by CMock)
}

void test_gateway_node_added_callback_should_call_process_send(void)
{
  // ARRANGE
  struct zigpc_gateway_on_node_add callback_data;

  // ACT
  zigpc_net_mgmt_callback_on_node_add_complete(&callback_data);

  // ASSERT (Handled by CMock)
}

void test_discovery_status_callback_should_call_process_send(void)
{
  zigbee_eui64_uint_t eui64 = 0x0D0E0A0D0B0E0E0F;

  // ACT
  zigpc_net_mgmt_on_discovery_status(eui64, DISCOVERY_START);

  zigpc_net_mgmt_on_discovery_status(eui64, DISCOVERY_SUCCESS);

  zigpc_net_mgmt_on_discovery_status(eui64, DEVICE_DISCOVERY_FAIL);
  zigpc_net_mgmt_on_discovery_status(eui64, ENDPOINT_DISCOVERY_FAIL);
}

void test_gateway_node_removed_callback_should_call_process_send(void)
{
  // ARRANGE
  zigpc_gateway_on_node_removed_t callback_data;

  // ACT
  zigpc_net_mgmt_callback_on_node_removed(&callback_data);

  // ASSERT (Handled by CMock)
}
