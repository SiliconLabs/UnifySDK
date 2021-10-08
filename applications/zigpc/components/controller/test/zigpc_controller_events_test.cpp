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

// Shared UIC includes
#include <unity.h>
#include <sl_status.h>
#include <dotdot_mqtt_mock.h>
#include <uic_mqtt_mock.h>

// ZigPC includes
#include <attribute_management_mock.h>
#include <zcl_definitions.h>
#include <zigpc_datastore_mock.h>
#include <zigpc_gateway_mock.h>
#include <zigpc_net_mgmt_mock.h>
#include <zigpc_ota_zigbee_mock.h>

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
  zigbee_eui64_t eui64 = {1, 2, 3, 4, 5, 6, 7, 8};

  // ARRANGE
  zigpc_net_mgmt_interview_node_ExpectAndReturn(eui64, SL_STATUS_OK);

  // ACT
  sl_status_t status = zigpc_ctrl::on_device_announced(eui64);

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

/**
 *
 * Stub data + handler for uic_mqtt_dotdot_publish_supported_commands
 *
 */

const char *stub_dotdot_pub_commands_unid;
zigbee_endpoint_id_t *stub_dotdot_pub_commands_ep;
void stub_dotdot_pub_commands(dotdot_unid_t unid,
                              dotdot_endpoint_id_t endpoint,
                              int cmock_num_calls)
{
  TEST_ASSERT_EQUAL_STRING(stub_dotdot_pub_commands_unid, unid);
  TEST_ASSERT_EQUAL(*stub_dotdot_pub_commands_ep, endpoint);
}

void helper_expect_endpoint_interviewed_actions(zigbee_eui64_t *dev,
                                                const char *unid,
                                                zigbee_endpoint_id_t *ep,
                                                size_t *cl_count,
                                                zcl_cluster_id_t *cl_list)
{
  zigpc_datastore_get_cluster_count_ExpectAndReturn(*dev,
                                                    *ep,
                                                    ZCL_CLUSTER_SERVER_SIDE,
                                                    *cl_count);
  for (size_t i = 0; i < *cl_count; i++) {
    zigpc_datastore_find_cluster_by_index_ExpectAndReturn(
      *dev,
      *ep,
      ZCL_CLUSTER_SERVER_SIDE,
      i,
      nullptr,
      SL_STATUS_OK);
    zigpc_datastore_find_cluster_by_index_IgnoreArg_cluster_id();
    zigpc_datastore_find_cluster_by_index_ReturnThruPtr_cluster_id(&cl_list[i]);

    zigpc_datastore_contains_cluster_ExpectAndReturn(
      nullptr,
      *ep,
      ZCL_CLUSTER_CLIENT_SIDE,
      ZIGPC_ZCL_CLUSTER_OTA_UPGRADE,
      cl_list[i] == ZIGPC_ZCL_CLUSTER_OTA_UPGRADE);
    // FIXME: Ignoring eui64 until CMock integration in Unity is fixed
    zigpc_datastore_contains_cluster_IgnoreArg_eui64();

    if (cl_list[i] == ZIGPC_ZCL_CLUSTER_OTA_UPGRADE) {
      zigpc_ota_configure_node_ExpectAndReturn(*dev, SL_STATUS_OK);
    }
  }

  stub_bind_request_dev      = dev;
  stub_bind_request_ep       = ep;
  stub_bind_request_cl_count = cl_count;
  stub_bind_request_cl_list  = cl_list;
  zigpc_gateway_request_binding_endpoint_StubWithCallback(stub_bind_request);

  stub_attr_report_request_dev      = dev;
  stub_attr_report_request_ep       = ep;
  stub_attr_report_request_cl_count = cl_count;
  stub_attr_report_request_cl_list  = cl_list;
  configure_attributes_endpoint_StubWithCallback(stub_attr_report_request);

  stub_dotdot_pub_commands_unid = unid;
  stub_dotdot_pub_commands_ep   = ep;
  uic_mqtt_dotdot_publish_supported_commands_StubWithCallback(
    stub_dotdot_pub_commands);
}

void test_endpoint_interviewed_sanity(void)
{
  zigbee_eui64_t dev      = {0x00, 0xAA, 0x42, 0x75, 0x5, 0x99, 0xD, 0x2};
  char unid[]             = "zb-00AA427505990D02";
  zigbee_endpoint_id_t ep = 1;
  size_t cl_count         = 3;
  zcl_cluster_id_t expected_cl[cl_count] = {0x004, 0x0005, 0x0029};

  // ARRANGE
  helper_expect_endpoint_interviewed_actions(&dev,
                                             unid,
                                             &ep,
                                             &cl_count,
                                             expected_cl);

  // ACT
  sl_status_t status = zigpc_ctrl::on_endpoint_interviewed(dev, ep);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_device_interviewed_sanity(void)
{
  zigbee_eui64_t dev = {0x00, 0xAA, 0x42, 0x75, 0x5, 0x99, 0xD, 0x2};
  char unid[]        = "zb-00AA427505990D02";

  size_t ep_count           = 1;
  zigbee_endpoint_id_t ep_1 = 1;
  size_t ep1_cl_count       = 5;
  zcl_cluster_id_t ep1_expected_cl[ep1_cl_count]
    = {0x0000, 0x0001, 0x0002, 0x0003, ZIGPC_ZCL_CLUSTER_OTA_UPGRADE};

  // ARRANGE
  zigpc_datastore_get_endpoint_count_ExpectAndReturn(dev, ep_count);
  zigpc_datastore_find_endpoint_by_index_ExpectAndReturn(dev,
                                                         0,
                                                         nullptr,
                                                         SL_STATUS_OK);
  zigpc_datastore_find_endpoint_by_index_IgnoreArg_endpoint_id();
  zigpc_datastore_find_endpoint_by_index_ReturnThruPtr_endpoint_id(&ep_1);

  helper_expect_endpoint_interviewed_actions(&dev,
                                             unid,
                                             &ep_1,
                                             &ep1_cl_count,
                                             ep1_expected_cl);

  // ACT
  sl_status_t status = zigpc_ctrl::on_device_interviewed(dev);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_controller_shutdown_sanity(void)
{
  zigpc_network_data_t nwk = {
    .gateway_eui64 = {5, 5, 5, 5, 0, 1, 0, 1},
  };
  size_t dev_count                       = 3;
  zigbee_eui64_t expected_dev[dev_count] = {
    {1, 0, 1, 0, 1, 0, 1, 0},
    {3, 0, 3, 0, 3, 0, 3, 0},
    {9, 0, 9, 0, 9, 0, 9, 0},
  };
  std::string expected_unretains[dev_count + 1] = {
    "ucl/by-unid/zb-0100010001000100",
    "ucl/by-unid/zb-0300030003000300",
    "ucl/by-unid/zb-0900090009000900",
    "ucl/by-unid/zb-0505050500010001",
  };

  // ARRANGE
  zigpc_datastore_get_device_count_ExpectAndReturn(dev_count);
  for (size_t i = 0; i < dev_count; i++) {
    zigpc_datastore_find_device_by_index_ExpectAndReturn(i, NULL, SL_STATUS_OK);
    zigpc_datastore_find_device_by_index_IgnoreArg_eui64();
    zigpc_datastore_find_device_by_index_ReturnThruPtr_eui64(&expected_dev[i]);
  }

  zigpc_datastore_read_network_ExpectAndReturn(NULL, SL_STATUS_OK);
  zigpc_datastore_read_network_IgnoreArg_data();
  zigpc_datastore_read_network_ReturnThruPtr_data(&nwk);

  for (size_t i = 0; i < dev_count + 1; i++) {
    uic_mqtt_unretain_Expect(expected_unretains[i].c_str());
  }

  // ACT
  zigpc_ctrl::on_shutdown();

  // ASSERT
}

}  // extern "C"
