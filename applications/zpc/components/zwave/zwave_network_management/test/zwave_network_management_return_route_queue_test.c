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
// Component being tested
#include "zwave_network_management_return_route_queue.h"
#include "zwave_network_management.h"
#include "zwave_network_management_fixt.h"
#include "zwave_network_management_process.h"
#include "zwave_network_management_callbacks.h"

// Test helpers and test includes
#include "unity.h"
#include "contiki_test_helper.h"

// Mock includes
#include "zwave_network_management_helpers_mock.h"
#include "zwave_s2_keystore_mock.h"
#include "zwave_s2_network_mock.h"
#include "zwave_s0_network_mock.h"
#include "zwapi_protocol_controller_mock.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  network_management_refresh_network_information_Expect();
  network_management_stop_ongoing_operations_Expect();
  network_management_take_sis_role_if_no_suc_in_network_Expect();
  zwave_s2_set_network_callbacks_Ignore();
  zwave_s0_set_network_callbacks_Ignore();
  zwave_s2_keystore_get_dsk_Ignore();
  zwave_network_management_fixt_setup();

  zwave_s2_set_network_callbacks_StopIgnore();
  zwave_s0_set_network_callbacks_StopIgnore();

  contiki_test_helper_init();
  process_start(&zwave_network_management_process, NULL);
}

void test_fill_and_clear_queue()
{
  assign_return_route_t request = {};
  request.node_id               = 1;
  request.destination_node_id   = 1;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_return_route_add_to_queue(&request));

  // Same request will just trigger an OK:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_return_route_add_to_queue(&request));

  // Fill up the queue:
  for (uint8_t i = 2; i <= ASSIGN_RETURN_ROUTE_QUEUE_SIZE; i++) {
    request.node_id = i;
    TEST_ASSERT_EQUAL(
      SL_STATUS_OK,
      zwave_network_management_return_route_add_to_queue(&request));
  }

  // Now there should be no more space:
  request.node_id = ASSIGN_RETURN_ROUTE_QUEUE_SIZE + 1;
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    zwave_network_management_return_route_add_to_queue(&request));

  // Duplicate request will still say OK:
  request.node_id = 1;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_return_route_add_to_queue(&request));

  // Clear the queue, now new request will work:
  // Duplicate request will still say OK:
  zwave_network_management_return_route_clear_queue();
  request.node_id = ASSIGN_RETURN_ROUTE_QUEUE_SIZE + 1;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_return_route_add_to_queue(&request));
}

void test_assign_next_route()
{
  // Ask to assign the next route, with an empty queue:
  TEST_ASSERT_EQUAL(SL_STATUS_IDLE,
                    zwave_network_management_return_route_assign_next());

  // Prepare 2 requests, and queue them:
  assign_return_route_t request = {};
  request.node_id               = 1;
  request.destination_node_id   = 1;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_return_route_add_to_queue(&request));

  request.node_id             = 2;
  request.destination_node_id = 2;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_return_route_add_to_queue(&request));

  // Now it should assign 1->1
  zwapi_assign_return_route_ExpectAndReturn(1,
                                            1,
                                            &on_assign_return_route_complete,
                                            SL_STATUS_OK);
  TEST_ASSERT_EQUAL(SL_STATUS_IN_PROGRESS,
                    zwave_network_management_return_route_assign_next());

  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(NM_ASSIGNING_RETURN_ROUTE,
                    zwave_network_management_get_state());

  // 1->1 is in progress, multiple calls should not do anything
  TEST_ASSERT_EQUAL(SL_STATUS_IN_PROGRESS,
                    zwave_network_management_return_route_assign_next());

  // When 1->1 is done, it should try 2->2
  zwapi_assign_return_route_ExpectAndReturn(2,
                                            2,
                                            &on_assign_return_route_complete,
                                            SL_STATUS_OK);
  on_assign_return_route_complete(0);

  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(NM_ASSIGNING_RETURN_ROUTE,
                    zwave_network_management_get_state());

  // Finally, when 2->2 is complete, it will post an event saying that we are done.
  on_assign_return_route_complete(0);
  TEST_ASSERT_EQUAL(NM_ASSIGNING_RETURN_ROUTE,
                    zwave_network_management_get_state());
  reset_nms_last_operation_data_Expect();
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_zwave_api_reject_request()
{
  // Prepare 3 requests, and queue them:
  assign_return_route_t request = {};
  request.node_id               = 1;
  request.destination_node_id   = 1;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_return_route_add_to_queue(&request));

  request.node_id             = 2;
  request.destination_node_id = 2;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_return_route_add_to_queue(&request));

  request.node_id             = 3;
  request.destination_node_id = 3;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_return_route_add_to_queue(&request));

  // Now it should assign 1->1, when we run the events.
  // Z-Wave API will reject the first request:
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
  zwapi_assign_return_route_ExpectAndReturn(1,
                                            1,
                                            &on_assign_return_route_complete,
                                            SL_STATUS_FAIL);
  zwapi_assign_return_route_ExpectAndReturn(2,
                                            2,
                                            &on_assign_return_route_complete,
                                            SL_STATUS_OK);
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(NM_ASSIGNING_RETURN_ROUTE,
                    zwave_network_management_get_state());

  TEST_ASSERT_EQUAL(SL_STATUS_IN_PROGRESS,
                    zwave_network_management_return_route_assign_next());

  // 1->1 is in progress, multiple calls should not do anything
  TEST_ASSERT_EQUAL(SL_STATUS_IN_PROGRESS,
                    zwave_network_management_return_route_assign_next());

  // When 2->2 is done, it should try 3->3, which will fail
  zwapi_assign_return_route_ExpectAndReturn(3,
                                            3,
                                            &on_assign_return_route_complete,
                                            SL_STATUS_FAIL);
  on_assign_return_route_complete(0);
  reset_nms_last_operation_data_Expect();
  contiki_test_helper_run(1);

  // 1->1 has just been assigned, it should be in cooldown, nothing happens.
  request.node_id             = 1;
  request.destination_node_id = 1;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_return_route_add_to_queue(&request));
  contiki_test_helper_run(1);

  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
  TEST_ASSERT_FALSE(we_have_return_routes_to_assign(1));

  // Now pass the cooldown and try to assign again
  contiki_test_helper_run(ASSIGN_RETURN_ROUTE_COOLDOWN);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_return_route_add_to_queue(&request));

  // Now it should assign 1->1, when we run the events.
  // Z-Wave API will reject the first request:
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
  zwapi_assign_return_route_ExpectAndReturn(1,
                                            1,
                                            &on_assign_return_route_complete,
                                            SL_STATUS_OK);
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(NM_ASSIGNING_RETURN_ROUTE,
                    zwave_network_management_get_state());
}

void test_zwave_network_management_abort()
{
  // Prepare 3 requests, and queue them:
  assign_return_route_t request = {};
  request.node_id               = 1;
  request.destination_node_id   = 1;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_return_route_add_to_queue(&request));

  request.node_id             = 2;
  request.destination_node_id = 1;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_return_route_add_to_queue(&request));

  request.node_id             = 1;
  request.destination_node_id = 2;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_return_route_add_to_queue(&request));

  // Now it should assign 1->1, when we run the events.
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
  zwapi_assign_return_route_ExpectAndReturn(1,
                                            1,
                                            &on_assign_return_route_complete,
                                            SL_STATUS_OK);
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(NM_ASSIGNING_RETURN_ROUTE,
                    zwave_network_management_get_state());

  // Now call Z-Wave network management abort (asking to return to IDLE)
  // It should not try to assign the rest of the return routes
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_network_management_abort());
  reset_nms_last_operation_data_Expect();
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(NM_IDLE, zwave_network_management_get_state());
}

void test_we_have_return_routes_to_assign()
{
  TEST_ASSERT_FALSE(we_have_return_routes_to_assign(1));
  TEST_ASSERT_FALSE(we_have_return_routes_to_assign(2));
  TEST_ASSERT_FALSE(we_have_return_routes_to_assign(3));

  // Queue a request
  assign_return_route_t request = {};
  request.node_id               = 1;
  request.destination_node_id   = 2;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_return_route_add_to_queue(&request));

  TEST_ASSERT_TRUE(we_have_return_routes_to_assign(1));
  TEST_ASSERT_FALSE(we_have_return_routes_to_assign(2));
  TEST_ASSERT_FALSE(we_have_return_routes_to_assign(3));

  // Queue another request
  request.node_id             = 2;
  request.destination_node_id = 3;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_network_management_return_route_add_to_queue(&request));

  TEST_ASSERT_TRUE(we_have_return_routes_to_assign(1));
  TEST_ASSERT_TRUE(we_have_return_routes_to_assign(2));
  TEST_ASSERT_FALSE(we_have_return_routes_to_assign(3));
}