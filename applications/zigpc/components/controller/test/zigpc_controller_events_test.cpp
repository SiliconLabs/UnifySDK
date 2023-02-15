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

// Shared Unify includes
#include <string.h>
#include <unity.h>
#include <sl_status.h>
#include "uic_mqtt_mock.h"

// ZigPC includes
#include <zcl_definitions.h>

// Mocks
#include "dotdot_mqtt_mock.h"
#include "attribute_management_mock.h"
#include "zigpc_datastore_mock.h"
#include "zigpc_discovery_mock.h"
#include "zigpc_gateway_mock.h"
#include "zigpc_group_mgmt_mock.h"
#include "zigpc_net_mgmt_mock.h"
#include "zigpc_ota_zigbee_mock.h"

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

void test_device_annouced_sanity(void)
{
  zigbee_eui64_t eui64            = {1, 2, 3, 4, 5, 6, 7, 8};
  zigpc_device_data_t eui64_state = {
    .network_status = ZIGBEE_NODE_STATUS_NODEID_ASSIGNED,
    .max_cmd_delay  = 1,
  };

  std::string topic = "ucl/by-unid/zb-0102030405060708/State";
  std::string payload
    = R"({"MaximumCommandDelay":1,"NetworkStatus":"Online interviewing","Security":"Zigbee Z3"})";

  // ARRANGE
  zigpc_discovery_interview_device_ExpectAndReturn(
    zigbee_eui64_to_uint(eui64),
    zigpc_ctrl::zigpc_discovery_update_callback,
    SL_STATUS_OK);

  zigpc_datastore_read_device_ExpectAndReturn(eui64, nullptr, SL_STATUS_OK);
  zigpc_datastore_read_device_IgnoreArg_data();
  zigpc_datastore_read_device_ReturnMemThruPtr_data(
    &eui64_state,
    sizeof(zigpc_device_data_t));

  uic_mqtt_publish_Expect(topic.c_str(), payload.c_str(), payload.size(), true);

  // ACT
  sl_status_t status = zigpc_ctrl::on_device_announce(eui64);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

/**
 *
 * Stub data + handler for zigpc_gateway_request_binding_endpoint()
 *
 */

zigbee_eui64_t *stub_bind_request_dev;
zigbee_endpoint_id_t *stub_bind_request_ep;
size_t *stub_bind_request_cl_count;
zcl_cluster_id_t *stub_bind_request_cl_list;
sl_status_t stub_bind_request(const zigbee_eui64_t eui64,
                              const zigbee_endpoint_t endpoint,
                              int cmock_num_calls)
{
  TEST_ASSERT_EQUAL_HEX8_ARRAY(*stub_bind_request_dev,
                               eui64,
                               sizeof(zigbee_eui64_t));
  TEST_ASSERT_EQUAL(*stub_bind_request_ep, endpoint.endpoint_id);
  TEST_ASSERT_EQUAL(*stub_bind_request_cl_count, endpoint.cluster_count);
  for (size_t i = 0; i < endpoint.cluster_count; i++) {
    TEST_ASSERT_EQUAL_HEX16(stub_bind_request_cl_list[i],
                            endpoint.cluster_list[i].cluster_id);
  }

  return SL_STATUS_OK;
}

/**
 *
 * Stub data + handler for configure_attributes_endpoint
 *
 */

zigbee_eui64_t *stub_attr_report_request_dev;
zigbee_endpoint_id_t *stub_attr_report_request_ep;
size_t *stub_attr_report_request_cl_count;
zcl_cluster_id_t *stub_attr_report_request_cl_list;
sl_status_t stub_attr_report_request(const zigbee_eui64_t eui64,
                                     const zigbee_endpoint_t endpoint,
                                     int cmock_num_calls)
{
  TEST_ASSERT_EQUAL_HEX8_ARRAY(*stub_attr_report_request_dev,
                               eui64,
                               sizeof(zigbee_eui64_t));
  TEST_ASSERT_EQUAL(*stub_attr_report_request_ep, endpoint.endpoint_id);
  TEST_ASSERT_EQUAL(*stub_attr_report_request_cl_count, endpoint.cluster_count);
  for (size_t i = 0; i < endpoint.cluster_count; i++) {
    TEST_ASSERT_EQUAL_HEX16(stub_attr_report_request_cl_list[i],
                            endpoint.cluster_list[i].cluster_id);
  }

  return SL_STATUS_OK;
}

void helper_expect_endpoint_configure_actions(zigbee_eui64_t dev,
                                              zigbee_endpoint_t &ep_obj)
{
    
    zigpc_network_data_t network_data;
  zigbee_eui64_t gw_eui64       = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1};
    memcpy(network_data.gateway_eui64,gw_eui64, sizeof(zigbee_eui64_t));
  zigpc_datastore_get_cluster_count_ExpectAndReturn(dev,
                                                    ep_obj.endpoint_id,
                                                    ZCL_CLUSTER_SERVER_SIDE,
                                                    ep_obj.cluster_count);
  for (size_t i = 0; i < ep_obj.cluster_count; i++) {
    zigpc_datastore_find_cluster_by_index_ExpectAndReturn(
      dev,
      ep_obj.endpoint_id,
      ZCL_CLUSTER_SERVER_SIDE,
      i,
      nullptr,
      SL_STATUS_OK);
    zigpc_datastore_find_cluster_by_index_IgnoreArg_cluster_id();
    zigpc_datastore_find_cluster_by_index_ReturnThruPtr_cluster_id(
      &ep_obj.cluster_list[i].cluster_id);
  }

  //zigpc_gateway_request_binding_endpoint_ExpectAndReturn(dev,
  //                                                       ep_obj,
  //                                                       gw_eui64,
  //                                                       SL_STATUS_OK);

  zigpc_gateway_request_binding_endpoint_IgnoreAndReturn(SL_STATUS_OK);
  configure_attributes_endpoint_ExpectAndReturn(dev, ep_obj, SL_STATUS_OK);
}

void helper_expect_endpoint_update_capabilities(zigbee_eui64_t dev,
                                                zigbee_endpoint_id_t ep,
                                                bool expect_ota)
{
  zigpc_datastore_contains_cluster_ExpectAndReturn(
    dev,
    ep,
    ZCL_CLUSTER_CLIENT_SIDE,
    ZIGPC_ZCL_CLUSTER_OTA_UPGRADE,
    expect_ota);

  if (expect_ota) {
    zigpc_ota_configure_node_ExpectAndReturn(dev, SL_STATUS_OK);
  }

  uic_mqtt_dotdot_publish_supported_commands_Expect(nullptr, ep);
  uic_mqtt_dotdot_publish_supported_commands_IgnoreArg_unid();
}

void test_endpoint_configuration_sanity(void)
{
  zigbee_eui64_t dev       = {0x00, 0xAA, 0x42, 0x75, 0x5, 0x99, 0xD, 0x2};
  zigbee_eui64_t gw_eui64       = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1};
  zigbee_endpoint_t ep_obj = {
    .endpoint_id = 1,
    .cluster_list
    = {{.cluster_id = 0x02}, {.cluster_id = 0x05}, {.cluster_id = 0x29}},
    .cluster_count = 3,
  };
  
    zigpc_network_data_t network_data;
    memcpy(network_data.gateway_eui64,gw_eui64, sizeof(zigbee_eui64_t));

  zigpc_datastore_find_cluster_by_index_ExpectAndReturn
      (dev,
      1,
      ZCL_CLUSTER_SERVER_SIDE,
      0,
      NULL,
      SL_STATUS_OK);
  zigpc_datastore_find_cluster_by_index_IgnoreArg_cluster_id();
  zigpc_datastore_find_cluster_by_index_ReturnThruPtr_cluster_id(&(ep_obj.cluster_list[0].cluster_id));
  
  zigpc_datastore_find_cluster_by_index_ExpectAndReturn
      (dev,
      1,
      ZCL_CLUSTER_SERVER_SIDE,
      1,
      NULL,
      SL_STATUS_OK);
  zigpc_datastore_find_cluster_by_index_IgnoreArg_cluster_id();
  zigpc_datastore_find_cluster_by_index_ReturnThruPtr_cluster_id(&(ep_obj.cluster_list[1].cluster_id));
    
  zigpc_datastore_find_cluster_by_index_ExpectAndReturn
      (dev,
      1,
      ZCL_CLUSTER_SERVER_SIDE,
      2,
      NULL,
      SL_STATUS_OK);
  zigpc_datastore_find_cluster_by_index_IgnoreArg_cluster_id();
  zigpc_datastore_find_cluster_by_index_ReturnThruPtr_cluster_id(&(ep_obj.cluster_list[2].cluster_id));
  
    zigpc_datastore_get_cluster_count_ExpectAndReturn(dev, ep_obj.endpoint_id, ZCL_CLUSTER_SERVER_SIDE, 3);

  zigpc_datastore_read_network_ExpectAndReturn(NULL, SL_STATUS_OK);
  zigpc_datastore_read_network_IgnoreArg_data();
  zigpc_datastore_read_network_ReturnThruPtr_data(&network_data);

  zigpc_gateway_request_binding_endpoint_ExpectAndReturn(dev,
                                                         ep_obj,
                                                         gw_eui64,
                                                         SL_STATUS_OK);

  configure_attributes_endpoint_ExpectAndReturn(dev, ep_obj, SL_STATUS_OK);
    
  uic_mqtt_publish_Ignore();
  uic_mqtt_subscribe_Ignore();
    
  zigpc_datastore_get_cluster_count_IgnoreAndReturn(0);

  // ACT
  sl_status_t status
    = zigpc_ctrl::perform_endpoint_configuration(dev, ep_obj.endpoint_id);
  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_endpoint_update_sanity(void)
{
  zigbee_eui64_t dev      = {0x00, 0xAA, 0x42, 0x75, 0x5, 0x99, 0xD, 0x12};
  zigbee_endpoint_id_t ep = 1;

  // ARRANGE
  helper_expect_endpoint_update_capabilities(dev, ep, false);
  zigpc_datastore_get_cluster_count_IgnoreAndReturn(0);

  // ACT
  sl_status_t status = zigpc_ctrl::update_endpoint_capabilities(dev, ep);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_device_update_capabilities_sanity(void)
{
  zigbee_eui64_t dev = {0x00, 0xAA, 0x42, 0x75, 0x5, 0x99, 0xD, 0x12};
  std::vector<zigbee_endpoint_id_t> ep_list = {1, 3, 4, 7};
  std::string base_topic("ucl/by-unid/zb-00AA427505990D12");

  // ARRANGE
  uic_mqtt_dotdot_state_endpoint_id_list_publish_ExpectAndReturn(
    base_topic.c_str(),
    ep_list.size(),
    ep_list.data(),
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);

  // ACT
  sl_status_t status = zigpc_ctrl::update_device_capabilities(dev, ep_list);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_device_interviewed_sanity(void)
{
  zigbee_eui64_t dev       = {0x00, 0xAA, 0x42, 0x75, 0x5, 0x99, 0xD, 0x2};
  size_t ep_count          = 1;
  zigbee_endpoint_t ep_obj = {
    .endpoint_id   = 1,
    .cluster_list  = {{.cluster_id = 0x0000},
                     {.cluster_id = 0x0001},
                     {.cluster_id = 0x0002},
                     {.cluster_id = 0x0003},
                     {.cluster_id = ZIGPC_ZCL_CLUSTER_OTA_UPGRADE}},
    .cluster_count = 5,
  };
  zigpc_device_data_t eui64_state = {
    .network_status = ZIGBEE_NODE_STATUS_INCLUDED,
    .max_cmd_delay  = 1,
  };
  std::string topic_unid = "ucl/by-unid/zb-00AA427505990D02";
  std::string topic      = topic_unid + "/State";
  std::string payload
    = R"({"MaximumCommandDelay":1,"NetworkStatus":"Online functional","Security":"Zigbee Z3"})";
  
  zigpc_network_data_t network_data;
  zigbee_eui64_t gw_eui64       = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1};
  memcpy(network_data.gateway_eui64,gw_eui64, sizeof(zigbee_eui64_t));
  zigpc_datastore_read_network_ExpectAndReturn(NULL, SL_STATUS_OK);
  zigpc_datastore_read_network_IgnoreArg_data();
  zigpc_datastore_read_network_ReturnThruPtr_data(&network_data);

  zigpc_datastore_read_device_ExpectAndReturn(dev, nullptr, SL_STATUS_OK);
  zigpc_datastore_read_device_IgnoreArg_data();
  zigpc_datastore_read_device_ReturnMemThruPtr_data(
    &eui64_state,
    sizeof(zigpc_device_data_t));

  uic_mqtt_publish_Expect(topic.c_str(), payload.c_str(), payload.size(), true);
  uic_mqtt_publish_Ignore();
  uic_mqtt_subscribe_Ignore();

  // ARRANGE
  zigpc_datastore_get_endpoint_count_ExpectAndReturn(dev, ep_count);
  zigpc_datastore_find_endpoint_by_index_ExpectAndReturn(dev,
                                                         0,
                                                         nullptr,
                                                         SL_STATUS_OK);
  zigpc_datastore_find_endpoint_by_index_IgnoreArg_endpoint_id();
  zigpc_datastore_find_endpoint_by_index_ReturnThruPtr_endpoint_id(
    &ep_obj.endpoint_id);

  helper_expect_endpoint_configure_actions(dev, ep_obj);
  helper_expect_endpoint_update_capabilities(dev, ep_obj.endpoint_id, true);

  uic_mqtt_dotdot_state_endpoint_id_list_publish_ExpectWithArrayAndReturn(
    topic_unid.c_str(),
    1,
    &ep_obj.endpoint_id,
    1,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);

  zigpc_datastore_get_cluster_count_IgnoreAndReturn(0);
  //zigpc_gateway_request_binding_endpoint
  // ACT
  sl_status_t status = zigpc_ctrl::on_device_interviewed(dev, true);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_device_leave_sanity(void)
{
  zigbee_eui64_t dev = {0x00, 0xAA, 0x42, 0x75, 0x5, 0x9A, 0xD, 0x2};
  size_t ep_count    = 2;
  zigbee_endpoint_id_t ep_list[ep_count] = {1, 3};

  std::string topic = "ucl/by-unid/zb-00AA4275059A0D02";

  // ARRANGE
  zigpc_datastore_get_endpoint_count_ExpectAndReturn(dev, ep_count);
  for (size_t i = 0; i < ep_count; i++) {
    zigpc_datastore_find_endpoint_by_index_ExpectAndReturn(dev,
                                                           i,
                                                           nullptr,
                                                           SL_STATUS_OK);
    zigpc_datastore_find_endpoint_by_index_IgnoreArg_endpoint_id();
    zigpc_datastore_find_endpoint_by_index_ReturnThruPtr_endpoint_id(
      &ep_list[i]);

    zigpc_group_remove_all_ExpectAndReturn(dev, ep_list[i], SL_STATUS_OK);
  }

  uic_mqtt_unretain_Expect(topic.c_str());

  // ACT
  sl_status_t status = zigpc_ctrl::on_device_leave(dev);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_controller_shutdown_sanity(void)
{
  size_t dev_count                       = 3;
  zigbee_eui64_t expected_dev[dev_count] = {
    {1, 0, 1, 0, 1, 0, 1, 0},
    {3, 0, 3, 0, 3, 0, 3, 0},
    {9, 0, 9, 0, 9, 0, 9, 0},
  };
  std::string expected_state_publish_topics[dev_count + 1] = {
    "ucl/by-unid/zb-0100010001000100/State",
    "ucl/by-unid/zb-0300030003000300/State",
    "ucl/by-unid/zb-0900090009000900/State",
    "ucl/by-unid/zb-0505050500010001/State",
  };
  std::string expected_state_publish_payload
    = R"({"MaximumCommandDelay":1,"NetworkStatus":"Unavailable","Security":"Zigbee Z3"})";
  zigpc_device_data_t dev_data = {
    .network_status = ZIGBEE_NODE_STATUS_INCLUDED,
    .max_cmd_delay  = 1,
  };

  // ARRANGE
  zigpc_datastore_get_device_count_ExpectAndReturn(dev_count);
  for (size_t i = 0; i < dev_count; i++) {
    zigpc_datastore_find_device_by_index_ExpectAndReturn(i, NULL, SL_STATUS_OK);
    zigpc_datastore_find_device_by_index_IgnoreArg_eui64();
    zigpc_datastore_find_device_by_index_ReturnThruPtr_eui64(&expected_dev[i]);

    zigpc_datastore_read_device_ExpectAndReturn(expected_dev[i],
                                                nullptr,
                                                SL_STATUS_OK);
    zigpc_datastore_read_device_IgnoreArg_data();
    zigpc_datastore_read_device_ReturnMemThruPtr_data(
      &dev_data,
      sizeof(zigpc_device_data_t));

    // expects parts of zigpc_ucl::node_state::publish_state call
    uic_mqtt_publish_Expect(expected_state_publish_topics[i].c_str(),
                            expected_state_publish_payload.c_str(),
                            expected_state_publish_payload.size(),
                            true);

    zigpc_datastore_write_device_ExpectAndReturn(expected_dev[i],
                                                 nullptr,
                                                 SL_STATUS_OK);
    zigpc_datastore_write_device_IgnoreArg_data();
  }

  uic_mqtt_unretain_by_regex_Expect(REGEX_NOT_STATE_OR_MQTT_CLIENT_TOPICS);

  // ACT
  zigpc_ctrl::on_shutdown();

  // ASSERT
}

void test_controller_shutdown_one_failed_publish(void)
{
  size_t dev_count                       = 3;
  size_t failed_dev_index                = 1;
  zigbee_eui64_t expected_dev[dev_count] = {
    {1, 0, 1, 0, 1, 0, 1, 0},
    {3, 0, 3, 0, 3, 0, 3, 0},
    {9, 0, 9, 0, 9, 0, 9, 0},
  };
  std::string expected_state_publish_topics[dev_count + 1] = {
    "ucl/by-unid/zb-0100010001000100/State",
    "ucl/by-unid/zb-0300030003000300/State",
    "ucl/by-unid/zb-0900090009000900/State",
    "ucl/by-unid/zb-0505050500010001/State",
  };
  std::string expected_state_publish_payload
    = R"({"MaximumCommandDelay":1,"NetworkStatus":"Unavailable","Security":"Zigbee Z3"})";
  zigpc_device_data_t dev_data = {
    .network_status = ZIGBEE_NODE_STATUS_INCLUDED,
    .max_cmd_delay  = 1,
  };

  // ARRANGE
  zigpc_datastore_get_device_count_ExpectAndReturn(dev_count);
  for (size_t i = 0; i < dev_count; i++) {
    zigpc_datastore_find_device_by_index_ExpectAndReturn(i, NULL, SL_STATUS_OK);
    zigpc_datastore_find_device_by_index_IgnoreArg_eui64();
    zigpc_datastore_find_device_by_index_ReturnThruPtr_eui64(&expected_dev[i]);

    zigpc_datastore_read_device_ExpectAndReturn(expected_dev[i],
                                                nullptr,
                                                SL_STATUS_OK);
    zigpc_datastore_read_device_IgnoreArg_data();
    zigpc_datastore_read_device_ReturnMemThruPtr_data(
      &dev_data,
      sizeof(zigpc_device_data_t));

    // Expect 1 device data write to fail, which should prevent the mqtt
    // publish
    if (i == failed_dev_index) {
      zigpc_datastore_write_device_ExpectAndReturn(expected_dev[i],
                                                   nullptr,
                                                   SL_STATUS_FAIL);

    } else {
      // expects parts of zigpc_ucl::node_state::publish_state call
      uic_mqtt_publish_Expect(expected_state_publish_topics[i].c_str(),
                              expected_state_publish_payload.c_str(),
                              expected_state_publish_payload.size(),
                              true);

      zigpc_datastore_write_device_ExpectAndReturn(expected_dev[i],
                                                   nullptr,
                                                   SL_STATUS_OK);
    }
    zigpc_datastore_write_device_IgnoreArg_data();
  }

  uic_mqtt_unretain_by_regex_Expect(REGEX_NOT_STATE_OR_MQTT_CLIENT_TOPICS);

  // ACT
  zigpc_ctrl::on_shutdown();

  // ASSERT
}

void test_controller_startup_sanity(void)
{
  size_t dev_count                       = 2;
  size_t ep_count                        = 1;
  zigbee_eui64_t expected_dev[dev_count] = {
    {0x2, 0x0, 0x1, 0x0, 0x1, 0xA, 0x1, 0x0},
    {0x1, 0x0, 0x3, 0x0, 0x3, 0xB, 0x3, 0xC},
  };
  zigbee_endpoint_id_t expected_eps[dev_count] = {
    2,
    1,
  };
  zigpc_device_data_t dev_data = {
    .network_status = ZIGBEE_NODE_STATUS_INCLUDED,
    .max_cmd_delay  = 1,
  };
  std::string expected_unids[dev_count] = {
    "ucl/by-unid/zb-02000100010A0100",
    "ucl/by-unid/zb-01000300030B030C",
  };
  std::string expected_state_publish_topics[dev_count] = {
    expected_unids[0] + "/State",
    expected_unids[1] + "/State",
  };
  std::string expected_state_publish_payload
    = R"({"MaximumCommandDelay":1,"NetworkStatus":"Online functional","Security":"Zigbee Z3"})";

  // ARRANGE
  uic_mqtt_dotdot_publish_supported_commands_StubWithCallback(nullptr);
  zigpc_datastore_get_device_count_ExpectAndReturn(dev_count);
  for (size_t i = 0; i < dev_count; i++) {
    zigpc_datastore_find_device_by_index_ExpectAndReturn(i,
                                                         nullptr,
                                                         SL_STATUS_OK);
    zigpc_datastore_find_device_by_index_IgnoreArg_eui64();
    zigpc_datastore_find_device_by_index_ReturnThruPtr_eui64(&expected_dev[i]);

    zigpc_datastore_read_device_ExpectAndReturn(expected_dev[i],
                                                nullptr,
                                                SL_STATUS_OK);
    zigpc_datastore_read_device_IgnoreArg_data();
    zigpc_datastore_write_device_ExpectAndReturn(expected_dev[i],
                                                 nullptr,
                                                 SL_STATUS_OK);
    zigpc_datastore_write_device_IgnoreArg_data();

    // expects parts of zigpc_ucl::node_state::on_status_update call
    zigpc_datastore_read_device_ExpectAndReturn(expected_dev[i],
                                                nullptr,
                                                SL_STATUS_OK);
    zigpc_datastore_read_device_IgnoreArg_data();
    zigpc_datastore_read_device_ReturnMemThruPtr_data(
      &dev_data,
      sizeof(zigpc_device_data_t));
    uic_mqtt_publish_Expect(expected_state_publish_topics[i].c_str(),
                            expected_state_publish_payload.c_str(),
                            expected_state_publish_payload.size(),
                            true);

    zigpc_datastore_get_endpoint_count_ExpectAndReturn(expected_dev[i],
                                                       ep_count);
    zigpc_datastore_find_endpoint_by_index_ExpectAndReturn(expected_dev[i],
                                                           0,
                                                           nullptr,
                                                           SL_STATUS_OK);
    zigpc_datastore_find_endpoint_by_index_IgnoreArg_endpoint_id();
    zigpc_datastore_find_endpoint_by_index_ReturnThruPtr_endpoint_id(
      &expected_eps[i]);

    // expect calls from endpoint enumeration
    helper_expect_endpoint_update_capabilities(expected_dev[i],
                                               expected_eps[i],
                                               true);

    uic_mqtt_dotdot_state_endpoint_id_list_publish_ExpectWithArrayAndReturn(
      expected_unids[i].c_str(),
      1,
      &expected_eps[i],
      1,
      UCL_MQTT_PUBLISH_TYPE_ALL,
      SL_STATUS_OK);
  }
  zigpc_datastore_get_cluster_count_IgnoreAndReturn(0);

  // ACT
  zigpc_ctrl::on_startup();

  // ASSERT
}

}  // extern "C"
