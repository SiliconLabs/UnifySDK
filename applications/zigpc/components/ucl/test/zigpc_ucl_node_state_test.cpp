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

#include "unity_cpp_includes.hpp"

extern "C" {

#include <unity.h>
#include <sl_status.h>

// Shared includes
#include <uic_mqtt_mock.h>

// ZigPC includes
#include <zigpc_datastore_mock.h>

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

void test_node_state_on_device_update_invalid_nwk_status(void)
{
  zigbee_eui64_uint_t eui64       = 0xD2BBCCDDEE001122;
  zigpc_device_data_t eui64_state = {
    .network_status = ZIGBEE_NODE_STATUS_INVALID,
    .max_cmd_delay  = 1,
  };

  std::string topic = "ucl/by-unid/zb-D2BBCCDDEE001122/State";
  std::string payload
    = R"({"MaximumCommandDelay":1,"NetworkStatus":"Unavailable","Security":"Zigbee Z3"})";

  // ARRANGE
  uic_mqtt_publish_Expect(topic.c_str(), payload.c_str(), payload.size(), true);

  // ACT
  sl_status_t status
    = zigpc_ucl::node_state::publish_state(eui64,
                                           eui64_state.network_status,
                                           eui64_state.max_cmd_delay);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_node_state_on_device_update_functional_nwk_status(void)
{
  zigbee_eui64_uint_t eui64       = 0xD2BBCCDDEE001122;
  zigpc_device_data_t eui64_state = {
    .network_status = ZIGBEE_NODE_STATUS_INCLUDED,
    .max_cmd_delay  = 2,
  };

  std::string topic = "ucl/by-unid/zb-D2BBCCDDEE001122/State";
  std::string payload
    = R"({"MaximumCommandDelay":2,"NetworkStatus":"Online functional","Security":"Zigbee Z3"})";

  // ARRANGE
  uic_mqtt_publish_Expect(topic.c_str(), payload.c_str(), payload.size(), true);

  // ACT
  sl_status_t status
    = zigpc_ucl::node_state::publish_state(eui64,
                                           eui64_state.network_status,
                                           eui64_state.max_cmd_delay);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

}  // extern "C"
