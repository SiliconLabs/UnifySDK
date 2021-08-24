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
#include "unity.h"
#include "ucl_nm_neighbor_discovery.h"
#include "ucl_nm_neighbor_discovery_test.hpp"

extern "C" {
#include "zwave_controller_callbacks_mock.h"
#include "zwave_network_management_mock.h"
#include "zwave_utils_mock.h"

const zwave_controller_callbacks_t *nm_neighbor_discovery_callbacks;
static void init_ucl_nm_neighbor_discovery();
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
  init_ucl_nm_neighbor_discovery();
}

static sl_status_t my_nm_callback_save(const zwave_controller_callbacks_t *cb,
                                       int n)
{
  nm_neighbor_discovery_callbacks = cb;
  return SL_STATUS_OK;
}

static void init_ucl_nm_neighbor_discovery()
{
  zwave_controller_register_callbacks_AddCallback(my_nm_callback_save);
  zwave_controller_register_callbacks_ExpectAndReturn(NULL, SL_STATUS_OK);
  zwave_controller_register_callbacks_IgnoreArg_callbacks();
  ucl_nm_neighbor_discovery_init();
}

void test_ucl_nm_trigger_node_neighbor_update()
{
  zwave_node_id_t node_id_test = 0x33;
  zwave_network_management_get_state_ExpectAndReturn(NM_IDLE);
  zwave_network_management_request_node_neighbor_discovery_ExpectAndReturn(
    node_id_test,
    SL_STATUS_OK);
  get_protocol_ExpectAndReturn(node_id_test, PROTOCOL_ZWAVE);
  ucl_nm_trigger_node_neighbor_update(node_id_test);
}

void test_ucl_nm_trigger_node_neighbor_update_with_cb_trigger()
{
  zwave_node_id_t node_id_test   = 0x33;
  zwave_node_id_t node_id_test_2 = 0x33;
  zwave_network_management_get_state_ExpectAndReturn(NM_WAITING_FOR_ADD);
  get_protocol_ExpectAndReturn(node_id_test, PROTOCOL_ZWAVE);
  ucl_nm_trigger_node_neighbor_update(node_id_test);

  zwave_network_management_get_state_ExpectAndReturn(NM_WAITING_FOR_ADD);
  get_protocol_ExpectAndReturn(node_id_test_2, PROTOCOL_ZWAVE);
  ucl_nm_trigger_node_neighbor_update(node_id_test_2);

  zwave_network_management_request_node_neighbor_discovery_ExpectAndReturn(
    node_id_test,
    SL_STATUS_OK);
  zwave_network_management_get_state_ExpectAndReturn(NM_IDLE);
  zwave_network_management_request_node_neighbor_discovery_ExpectAndReturn(
    node_id_test_2,
    SL_STATUS_OK);
  zwave_network_management_get_state_ExpectAndReturn(NM_IDLE);
  TEST_ASSERT_NOT_NULL(nm_neighbor_discovery_callbacks->on_state_updated);
  nm_neighbor_discovery_callbacks->on_state_updated(NM_IDLE);
}

void test_ucl_nm_trigger_node_neighbor_update_long_range_node()
{
  zwave_node_id_t node_id_test = 0x100;
  get_protocol_ExpectAndReturn(node_id_test, PROTOCOL_ZWAVE_LONG_RANGE);
  ucl_nm_trigger_node_neighbor_update(node_id_test);
}
}