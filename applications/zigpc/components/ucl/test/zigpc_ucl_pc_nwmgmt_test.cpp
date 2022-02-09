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

#include "zigpc_datastore_mock.h"
#include "zigpc_net_mgmt_mock.h"
#include "uic_mqtt_mock.h"

// Stubs for state_change API in network_managment

static zigpc_net_mgmt_state_change_request_t change_req_stub_data;
static int change_req_stub_calls = 0;

sl_status_t net_mgmt_state_change_request_stub(
  const zigpc_net_mgmt_state_change_request_t *request, int cmock_num_calls)
{
  change_req_stub_calls = cmock_num_calls + 1;

  change_req_stub_data = *request;

  return SL_STATUS_OK;
}

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
  change_req_stub_calls = 0;
  zigpc_net_mgmt_mock_Init();
}

/**
 * @brief Teardown after each test case
 *
 */
void tearDown(void)
{
  zigpc_net_mgmt_mock_Destroy();
}

void test_write_handler_should_reject_invalid_payloads(void)
{
  // ARRANGE
  std::string empty_message          = "";
  std::string invalid_json           = "{";
  std::string empty_json             = "{}";
  std::string wrong_fields_json      = R"({"Test": 2})";
  std::string wrong_state_field_json = R"({"State": "Testing"})";

  // ACT
  zigpc_ucl::pc_nwmgmt::on_write_mqtt(nullptr, nullptr, 0);
  // ASSERT
  TEST_ASSERT_EQUAL(0, change_req_stub_calls);

  // ACT
  zigpc_ucl::pc_nwmgmt::on_write_mqtt(nullptr,
                                      empty_message.c_str(),
                                      empty_message.size());
  // ASSERT
  TEST_ASSERT_EQUAL(0, change_req_stub_calls);

  // ACT
  zigpc_ucl::pc_nwmgmt::on_write_mqtt(nullptr,
                                      invalid_json.c_str(),
                                      invalid_json.size());
  // ASSERT
  TEST_ASSERT_EQUAL(0, change_req_stub_calls);

  // ACT
  zigpc_ucl::pc_nwmgmt::on_write_mqtt(nullptr,
                                      empty_json.c_str(),
                                      empty_json.size());
  // ASSERT
  TEST_ASSERT_EQUAL(0, change_req_stub_calls);

  // ACT
  zigpc_ucl::pc_nwmgmt::on_write_mqtt(nullptr,
                                      wrong_fields_json.c_str(),
                                      wrong_fields_json.size());
  // ASSERT
  TEST_ASSERT_EQUAL(0, change_req_stub_calls);

  // ACT
  zigpc_ucl::pc_nwmgmt::on_write_mqtt(nullptr,
                                      wrong_state_field_json.c_str(),
                                      wrong_state_field_json.size());
  // ASSERT
  TEST_ASSERT_EQUAL(0, change_req_stub_calls);
}

void test_write_handler_should_handle_valid_state_with_no_state_parameters(void)
{
  // ARRANGE
  std::string payload = R"({"State":"idle"})";

  zigpc_net_mgmt_state_change_request_StubWithCallback(
    net_mgmt_state_change_request_stub);

  // ACT
  zigpc_ucl::pc_nwmgmt::on_write_mqtt(nullptr, payload.c_str(), payload.size());

  // ASSERT
  TEST_ASSERT_EQUAL(1, change_req_stub_calls);
  TEST_ASSERT_EQUAL(ZIGPC_NET_MGMT_FSM_STATE_IDLE,
                    change_req_stub_data.requested_state);
  TEST_ASSERT_EQUAL(false, change_req_stub_data.param_eui64_filled);
}

void test_write_handler_should_handle_valid_state_with_empty_state_parameters(
  void)
{
  // ARRANGE
  std::string payload = R"({"State":"idle","StateParameters": {}})";

  zigpc_net_mgmt_state_change_request_StubWithCallback(
    net_mgmt_state_change_request_stub);

  // ACT
  zigpc_ucl::pc_nwmgmt::on_write_mqtt(nullptr, payload.c_str(), payload.size());

  // ASSERT
  TEST_ASSERT_EQUAL(1, change_req_stub_calls);
  TEST_ASSERT_EQUAL(ZIGPC_NET_MGMT_FSM_STATE_IDLE,
                    change_req_stub_data.requested_state);
  TEST_ASSERT_EQUAL(false, change_req_stub_data.param_eui64_filled);
}

void test_write_handler_should_accept_valid_state_with_known_unid(void)
{
  // ARRANGE
  zigbee_eui64_t eui64 = {0x01, 0xAA, 0x02, 0xBB, 0x03, 0xCC, 0x04, 0xDD};
  std::string payload
    = R"({"State":"remove node","StateParameters": {"Unid": "zb-01AA02BB03CC04DD"}})";

  zigpc_net_mgmt_state_change_request_StubWithCallback(
    net_mgmt_state_change_request_stub);

  // ACT
  zigpc_ucl::pc_nwmgmt::on_write_mqtt(nullptr, payload.c_str(), payload.size());

  // ASSERT
  TEST_ASSERT_EQUAL(1, change_req_stub_calls);
  TEST_ASSERT_EQUAL(ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE,
                    change_req_stub_data.requested_state);
  TEST_ASSERT_EQUAL(true, change_req_stub_data.param_eui64_filled);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(eui64,
                               change_req_stub_data.param_eui64,
                               ZIGBEE_EUI64_SIZE);
}

void test_write_handler_should_accept_valid_state_with_known_unid_others(void)
{
  // ARRANGE
  zigbee_eui64_t eui64 = {0x01, 0xAA, 0x02, 0xBB, 0x03, 0xCC, 0x04, 0xDD};
  std::string payload
    = R"({"State":"remove node","StateParameters": {"Testing":"Not Used","Unid": "zb-01AA02BB03CC04DD"}})";

  zigpc_net_mgmt_state_change_request_StubWithCallback(
    net_mgmt_state_change_request_stub);

  // ACT
  zigpc_ucl::pc_nwmgmt::on_write_mqtt(nullptr, payload.c_str(), payload.size());

  // ASSERT
  TEST_ASSERT_EQUAL(1, change_req_stub_calls);
  TEST_ASSERT_EQUAL(ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE,
                    change_req_stub_data.requested_state);
  TEST_ASSERT_EQUAL(true, change_req_stub_data.param_eui64_filled);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(eui64,
                               change_req_stub_data.param_eui64,
                               ZIGBEE_EUI64_SIZE);
}

void test_write_handler_should_reject_valid_state_with_unknown_unid(void)
{
  // ARRANGE
  std::string payload
    = R"({"State":"remove node","StateParameters": {"Unid": "zb-12345"}})";

  zigpc_net_mgmt_state_change_request_StubWithCallback(
    net_mgmt_state_change_request_stub);

  // ACT
  zigpc_ucl::pc_nwmgmt::on_write_mqtt(nullptr, payload.c_str(), payload.size());

  // ASSERT
  TEST_ASSERT_EQUAL(0, change_req_stub_calls);
}

void test_publish_state_should_publish_state_only(void)
{
  // ARRANGE

  zigpc_net_mgmt_fsm_state_t new_state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;
  zigpc_net_mgmt_on_network_state_update_t net_state = {
    .new_state                       = new_state,
    .next_supported_states_count     = 0,
    .requested_state_parameter_count = 0,
  };
  std::string topic
    = "ucl/by-unid/zb-AABBCCDDEE001122/ProtocolController/NetworkManagement";
  std::string payload
    = R"({"RequestedStateParameters":[],"State":"idle","SupportedStateList":[]})";

  zigpc_network_data_t nwk_data = {
    .gateway_eui64 = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x00, 0x11, 0x22},
  };
  zigpc_datastore_read_network_ExpectAndReturn(nullptr, SL_STATUS_OK);
  zigpc_datastore_read_network_IgnoreArg_data();
  zigpc_datastore_read_network_ReturnMemThruPtr_data(
    &nwk_data,
    sizeof(zigpc_network_data_t));

  uic_mqtt_publish_Expect(topic.c_str(), payload.c_str(), payload.size(), true);

  // ACT
  sl_status_t status = zigpc_ucl::pc_nwmgmt::on_net_state_update(net_state);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_publish_state_should_publish_next_states(void)
{
  // ARRANGE

  zigpc_net_mgmt_fsm_state_t new_state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;
  zigpc_net_mgmt_on_network_state_update_t net_state = {
    .new_state                       = new_state,
    .next_supported_states_list = {
      ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE,
      ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD,
    },
    .next_supported_states_count     = 2,
    .requested_state_parameter_count = 0,
  };
  std::string topic
    = "ucl/by-unid/zb-AABBCCDDEE001122/ProtocolController/NetworkManagement";
  std::string payload
    = R"({"RequestedStateParameters":[],"State":"idle","SupportedStateList":["remove node","add node"]})";

  zigpc_network_data_t nwk_data = {
    .gateway_eui64 = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x00, 0x11, 0x22},
  };
  zigpc_datastore_read_network_ExpectAndReturn(nullptr, SL_STATUS_OK);
  zigpc_datastore_read_network_IgnoreArg_data();
  zigpc_datastore_read_network_ReturnMemThruPtr_data(
    &nwk_data,
    sizeof(zigpc_network_data_t));

  uic_mqtt_publish_Expect(topic.c_str(), payload.c_str(), payload.size(), true);

  // ACT
  sl_status_t status = zigpc_ucl::pc_nwmgmt::on_net_state_update(net_state);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_publish_state_should_publish_requested_params(void)
{
  // ARRANGE

  zigpc_net_mgmt_fsm_state_t new_state = ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE;
  const char *req_params[]             = {
    "Unid",
    "ExtraFlag",
  };
  zigpc_net_mgmt_on_network_state_update_t net_state = {
    .new_state                       = new_state,
    .next_supported_states_list = {
      ZIGPC_NET_MGMT_FSM_STATE_IDLE,
    },
    .next_supported_states_count     = 1,
    .requested_state_parameter_list = {
      req_params[0],
      req_params[1],
    },
    .requested_state_parameter_count = 2,
  };
  std::string topic
    = "ucl/by-unid/zb-AABBCCDDEE001122/ProtocolController/NetworkManagement";
  std::string payload
    = R"({"RequestedStateParameters":["Unid","ExtraFlag"],"State":"remove node","SupportedStateList":["idle"]})";

  zigpc_network_data_t nwk_data = {
    .gateway_eui64 = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x00, 0x11, 0x22},
  };
  zigpc_datastore_read_network_ExpectAndReturn(nullptr, SL_STATUS_OK);
  zigpc_datastore_read_network_IgnoreArg_data();
  zigpc_datastore_read_network_ReturnMemThruPtr_data(
    &nwk_data,
    sizeof(zigpc_network_data_t));

  uic_mqtt_publish_Expect(topic.c_str(), payload.c_str(), payload.size(), true);

  // ACT
  sl_status_t status = zigpc_ucl::pc_nwmgmt::on_net_state_update(net_state);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

}  // extern "C"
