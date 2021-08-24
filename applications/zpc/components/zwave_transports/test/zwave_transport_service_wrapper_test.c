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
#include "zwave_transport_service.h"
#include "zwave_controller_internal_mock.h"
#include "zwave_controller_transport_mock.h"
#include "transport_service_mock.h"
#include "ZW_classcmd.h"
#include "zwave_controller_utils_mock.h"

static zwave_controller_transport_t ts_transport             = {};
sl_status_t zwave_controller_transport_register_stub(
  const zwave_controller_transport_t *transport, int cmock_num_calls)
{
  TEST_ASSERT_EQUAL(COMMAND_CLASS_TRANSPORT_SERVICE_V2, transport->command_class);
  TEST_ASSERT_EQUAL(2, transport->version);
  TEST_ASSERT_EQUAL(1, transport->priority);

  // Save the transport for our tests
  ts_transport = *transport;

  return SL_STATUS_OK;
}

uint8_t data[68] = {0x55, 0xC0, 3, 4, 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                    1,    2,    3, 4, 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                    1,    2,    3, 4, 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                    1,    2,    3, 4, 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

// Keep this test first
void test_zwave_multi_channel_transport_init()
{
  // Nothing to test here really. We intercept the registered transport.
  zwave_controller_transport_register_AddCallback(
    zwave_controller_transport_register_stub);

  zwave_controller_transport_register_ExpectAndReturn(NULL, SL_STATUS_OK);
  zwave_controller_transport_register_IgnoreArg_transport();
  transport_service_init_Expect(0, 0, 0, 0);
  transport_service_init_IgnoreArg_max_fragment_size();
  transport_service_init_IgnoreArg_my_node_id();
  transport_service_init_IgnoreArg_send_data();
  transport_service_init_IgnoreArg_upper_layer_command_handler();
  // Call the function
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_transport_service_transport_init());
}

void test_zwave_transport_service_on_frame_received_singlecast()
{
  zwave_controller_connection_info_t connection_info;
  zwave_rx_receive_options_t rx_options;

  transport_service_on_frame_received_ExpectAndReturn(1, 2, SINGLECAST, data,
                                                      sizeof(data), 1);

  connection_info.local.node_id = 2;
  connection_info.remote.node_id = 1;
  rx_options.status_flags = RECEIVE_STATUS_TYPE_SINGLE; 

  TEST_ASSERT_EQUAL(zwave_transport_service_on_frame_received(&connection_info,
                                            &rx_options,
                                            data,
                                            sizeof(data)), SL_STATUS_OK);
  uint8_t bkup = data[1];
  data[1] = COMMAND_SEGMENT_COMPLETE_V2; 
  transport_service_on_frame_received_ExpectAndReturn(1, 2, SINGLECAST, data,
                                                      sizeof(data), 1);
  TEST_ASSERT_EQUAL(zwave_transport_service_on_frame_received(&connection_info,
                                            &rx_options,
                                            data,
                                            sizeof(data)), SL_STATUS_OK);
  data[1] = COMMAND_SEGMENT_REQUEST_V2; 
  transport_service_on_frame_received_ExpectAndReturn(1, 2, SINGLECAST, data,
                                                      sizeof(data), 1);
  TEST_ASSERT_EQUAL(zwave_transport_service_on_frame_received(&connection_info,
                                            &rx_options,
                                            data,
                                            sizeof(data)), SL_STATUS_OK);
  data[1] = COMMAND_SEGMENT_WAIT_V2; 
  transport_service_on_frame_received_ExpectAndReturn(1, 2, SINGLECAST, data,
                                                      sizeof(data), 1);
  TEST_ASSERT_EQUAL(zwave_transport_service_on_frame_received(&connection_info,
                                            &rx_options,
                                            data,
                                            sizeof(data)), SL_STATUS_OK);
  data[1] = COMMAND_SUBSEQUENT_SEGMENT_V2; 
  transport_service_on_frame_received_ExpectAndReturn(1, 2, SINGLECAST, data,
                                                      sizeof(data), 1);
  TEST_ASSERT_EQUAL(zwave_transport_service_on_frame_received(&connection_info,
                                            &rx_options,
                                            data,
                                            sizeof(data)), SL_STATUS_OK);
  data[1] = 0; 
  TEST_ASSERT_EQUAL(zwave_transport_service_on_frame_received(&connection_info,
                                            &rx_options,
                                            data,
                                            sizeof(data)), SL_STATUS_NOT_FOUND);
  data[1] = bkup;
  bkup = data[0];
  data[0] = 0;
  TEST_ASSERT_EQUAL(zwave_transport_service_on_frame_received(&connection_info,
                                                              &rx_options,
                                                              data,
                                                              sizeof(data)),
                    SL_STATUS_NOT_SUPPORTED);
  data[0] = bkup;
}

void test_zwave_transport_service_on_frame_received_broadcast()
{
  zwave_controller_connection_info_t connection_info;
  zwave_rx_receive_options_t rx_options;

  transport_service_on_frame_received_ExpectAndReturn(1, 2, BROADCAST, data,
                                                      sizeof(data), 1);

  connection_info.local.node_id = 2;
  connection_info.remote.node_id = 1;
  rx_options.status_flags = RECEIVE_STATUS_TYPE_BROAD; 

  TEST_ASSERT_EQUAL(zwave_transport_service_on_frame_received(&connection_info,
                                            &rx_options,
                                            data,
                                            sizeof(data)), SL_STATUS_OK);
}

void test_zwave_transport_service_on_frame_received_multicast()
{
  zwave_controller_connection_info_t connection_info;
  zwave_rx_receive_options_t rx_options;

  transport_service_on_frame_received_ExpectAndReturn(1, 2, MULTICAST, data,
                                                      sizeof(data), true);

  connection_info.local.node_id = 2;
  connection_info.remote.node_id = 1;
  rx_options.status_flags = RECEIVE_STATUS_TYPE_MULTI; 

  TEST_ASSERT_EQUAL(zwave_transport_service_on_frame_received(&connection_info,
                                            &rx_options,
                                            data,
                                            sizeof(data)), SL_STATUS_OK);
}

void test_zwave_transport_service_on_frame_received_multicast_failure()
{
  zwave_controller_connection_info_t connection_info;
  zwave_rx_receive_options_t rx_options;

  transport_service_on_frame_received_ExpectAndReturn(1, 2, MULTICAST, data,
                                                      sizeof(data), false);

  connection_info.local.node_id = 2;
  connection_info.remote.node_id = 1;
  rx_options.status_flags = RECEIVE_STATUS_TYPE_MULTI; 

  TEST_ASSERT_EQUAL(zwave_transport_service_on_frame_received(&connection_info,
                                            &rx_options,
                                            data,
                                            sizeof(data)), SL_STATUS_FAIL);
}

void test_zwave_transport_service_send_data()
{
  zwave_controller_connection_info_t connection_info;
  zwave_tx_options_t tx_options = {};

  zwave_node_get_command_class_version_ExpectAndReturn(0, 0, 0, 2);
  zwave_node_get_command_class_version_IgnoreArg_command_class();
  zwave_node_get_command_class_version_IgnoreArg_endpoint_id();
  zwave_node_get_command_class_version_IgnoreArg_node_id();
  transport_service_send_data_ExpectAndReturn(1, 2, data, sizeof(data), 0, 
                                              TRANSPORT_SERVICE_SEND_SUCCESS);
  transport_service_send_data_IgnoreArg_on_send_complete();

  connection_info.local.node_id = 1;
  connection_info.remote.node_id = 2;
  TEST_ASSERT_EQUAL(zwave_transport_service_send_data(&connection_info,
                                                      sizeof(data),
                                                      data,
                                                      &tx_options,
                                                      0,
                                                      0,
                                                      0),
                    SL_STATUS_OK);
}

void test_zwave_transport_service_send_data_overflow()
{
  zwave_controller_connection_info_t connection_info;
  zwave_tx_options_t tx_options = {};

  zwave_node_get_command_class_version_ExpectAndReturn(0, 0, 0, 2);
  zwave_node_get_command_class_version_IgnoreArg_command_class();
  zwave_node_get_command_class_version_IgnoreArg_endpoint_id();
  zwave_node_get_command_class_version_IgnoreArg_node_id();
  transport_service_send_data_ExpectAndReturn(1, 2, data, sizeof(data), 0, 
                                              TRANSPORT_SERVICE_WILL_OVERFLOW);
  transport_service_send_data_IgnoreArg_on_send_complete();

  connection_info.local.node_id = 1;
  connection_info.remote.node_id = 2;
  TEST_ASSERT_EQUAL(zwave_transport_service_send_data(&connection_info,
                                                      sizeof(data),
                                                      data,
                                                      &tx_options,
                                                      0,
                                                      0,
                                                      0),
                    SL_STATUS_WOULD_OVERFLOW);
}

void test_zwave_transport_service_send_data_fail()
{
  zwave_controller_connection_info_t connection_info;
  zwave_tx_options_t tx_options = {};

  zwave_node_get_command_class_version_ExpectAndReturn(0, 0, 0, 2);
  zwave_node_get_command_class_version_IgnoreArg_command_class();
  zwave_node_get_command_class_version_IgnoreArg_endpoint_id();
  zwave_node_get_command_class_version_IgnoreArg_node_id();
  transport_service_send_data_ExpectAndReturn(1, 2, data, sizeof(data), 0, 
                                              TRANSPORT_SERVICE_SEND_FAILURE);
  transport_service_send_data_IgnoreArg_on_send_complete();

  connection_info.local.node_id = 1;
  connection_info.remote.node_id = 2;
  TEST_ASSERT_EQUAL(zwave_transport_service_send_data(&connection_info,
                                                      sizeof(data),
                                                      data,
                                                      &tx_options,
                                                      0,
                                                      0,
                                                      0),
                    SL_STATUS_FAIL);
}

void test_zwave_transport_service_send_data_cc_not_supported()
{
  zwave_controller_connection_info_t connection_info;
  zwave_tx_options_t tx_options = {};

  zwave_node_get_command_class_version_ExpectAndReturn(0, 0, 0, 0);
  zwave_node_get_command_class_version_IgnoreArg_command_class();
  zwave_node_get_command_class_version_IgnoreArg_endpoint_id();
  zwave_node_get_command_class_version_IgnoreArg_node_id();

  connection_info.local.node_id = 1;
  connection_info.remote.node_id = 2;
  TEST_ASSERT_EQUAL(zwave_transport_service_send_data(&connection_info,
                                                      sizeof(data),
                                                      data,
                                                      &tx_options,
                                                      0,
                                                      0,
                                                      0),
                    SL_STATUS_NOT_SUPPORTED);
}


