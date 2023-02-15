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
#include "smart_start_list_monitor.h"
#include "unity.h"

// C++ mocks.
#include "workaround.hpp"

using namespace smartstart;

extern "C" {

// Unify Components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_fixt.h"
#include "attribute_store_helper.h"

// ZPC Components
#include "attribute_store_defined_attribute_types.h"

// Helper includes
#include "zpc_attribute_store_test_helper.h"
#include "contiki_test_helper.h"

// Mock includes
#include "zwave_controller_callbacks_mock.h"
#include "zwave_network_management_mock.h"

// Constants
const std::string MY_UNID = "zw-CAFECAFE-001";

// Private test variables
static zwave_controller_reset_step_t smart_start_list_monitor_reset_step = NULL;

// Stub functions
static sl_status_t zwave_controller_register_reset_step_stub(
  zwave_controller_reset_step_t step_function,
  int32_t priority,
  int cmock_num_calls)
{
  TEST_ASSERT_NOT_NULL(step_function);
  smart_start_list_monitor_reset_step = step_function;
  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_SMART_START_LIST_UPDATE_STEP_PRIORITY,
                    priority);
  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_set_home_id(home_id);
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);
  zwave_network_management_get_node_id_IgnoreAndReturn(zpc_node_id);
  smart_start_list_monitor_reset_step = NULL;

  contiki_test_helper_init();
  Management::get_instance()->init(MY_UNID, NULL);

  zwave_controller_register_reset_step_Stub(
    &zwave_controller_register_reset_step_stub);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, smart_start_list_monitor_init());
}
void tearDown()
{
  Management::get_instance()->teardown();
}

void test_smart_start_list_monitor_test_node_no_dsk()
{
  TEST_ASSERT_NOT_NULL(smart_start_list_monitor_reset_step);

  // We have 1 node in the network with the zpc_attribute_store_test_helper.
  // No DSK for that node
  notify_node_removed_return_value = SL_STATUS_FAIL;
  TEST_ASSERT_NOT_EQUAL(SL_STATUS_OK, smart_start_list_monitor_reset_step());

  TEST_ASSERT_TRUE(notify_node_removed_called);
  TEST_ASSERT_FALSE(set_manual_intervention_required_called);
  TEST_ASSERT_FALSE(notify_node_added_called);
}

void test_smart_start_list_monitor_test_node_with_dsk_not_in_list()
{
  TEST_ASSERT_NOT_NULL(smart_start_list_monitor_reset_step);

  // We have 1 node in the network with the zpc_attribute_store_test_helper.
  // Add a DSK for that node
  attribute_store_node_t dsk_node
    = attribute_store_add_node(ATTRIBUTE_S2_DSK, node_id_node);
  zwave_dsk_t dsk = {0x00,
                     0x01,
                     0x00,
                     0x01,
                     0x00,
                     0x01,
                     0x00,
                     0x01,
                     0x00,
                     0x01,
                     0x00,
                     0x01,
                     0x00,
                     0x01,
                     0x00,
                     0x01};
  attribute_store_set_reported(dsk_node, dsk, sizeof(dsk));

  notify_node_removed_return_value = SL_STATUS_NOT_FOUND;
  TEST_ASSERT_NOT_EQUAL(SL_STATUS_OK, smart_start_list_monitor_reset_step());

  TEST_ASSERT_TRUE(notify_node_removed_called);
  TEST_ASSERT_FALSE(set_manual_intervention_required_called);
  TEST_ASSERT_FALSE(notify_node_added_called);

  TEST_ASSERT_EQUAL_STRING("zw-CAFECAFE-0004", received_device_unid.c_str());
}

void test_smart_start_list_monitor_test_node_with_dsk_in_list()
{
  TEST_ASSERT_NOT_NULL(smart_start_list_monitor_reset_step);

  // We have 1 node in the network with the zpc_attribute_store_test_helper.
  // Add a DSK for that node
  attribute_store_node_t dsk_node
    = attribute_store_add_node(ATTRIBUTE_S2_DSK, node_id_node);
  zwave_dsk_t dsk = {0x00,
                     0x01,
                     0x00,
                     0x01,
                     0x00,
                     0x01,
                     0x00,
                     0x01,
                     0x00,
                     0x01,
                     0x00,
                     0x01,
                     0x00,
                     0x01,
                     0x00,
                     0x01};
  attribute_store_set_reported(dsk_node, dsk, sizeof(dsk));

  notify_node_removed_return_value = SL_STATUS_OK;
  TEST_ASSERT_NOT_EQUAL(SL_STATUS_OK, smart_start_list_monitor_reset_step());

  TEST_ASSERT_TRUE(notify_node_removed_called);
  TEST_ASSERT_TRUE(set_manual_intervention_required_called);
  TEST_ASSERT_FALSE(notify_node_added_called);

  TEST_ASSERT_EQUAL_STRING("00001-00001-00001-00001-00001-00001-00001-00001",
                           received_dsk.c_str());
}

}  // extern "C"
