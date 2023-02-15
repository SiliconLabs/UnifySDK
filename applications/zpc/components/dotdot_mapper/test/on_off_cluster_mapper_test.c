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
#include "on_off_cluster_basic_mapper.h"
#include "unity.h"

// Unify components
#include "datastore.h"
#include "attribute_store_fixt.h"
#include "attribute_store_helper.h"
#include "unify_dotdot_defined_attribute_types.h"

// ZPC Components
#include "zwave_unid.h"

// Mock includes
#include "dotdot_mqtt_mock.h"
#include "zwave_network_management_mock.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"
#include "attribute_store_defined_attribute_types.h"

// Private variables
static unid_t node_unid;

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
  // Call init
  TEST_ASSERT_TRUE(on_off_cluster_basic_mapper_init());

  // Create a test network
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_from_node_id(node_id, node_unid);
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);
}

/// Called after each and every test
void tearDown()
{
  attribute_store_delete_node(attribute_store_get_root());
}

void test_on_onoff_attribute_created_due_to_basic_value()
{
  // set the network status of
  attribute_store_node_t network_status_node
    = attribute_store_add_node(ATTRIBUTE_NETWORK_STATUS, node_id_node);
  uint8_t test_online_network_status = 0x01;
  attribute_store_set_node_attribute_value(network_status_node,
                                           REPORTED_ATTRIBUTE,
                                           &test_online_network_status,
                                           sizeof(test_online_network_status));
  uic_mqtt_dotdot_publish_supported_commands_Expect(NULL, endpoint_id);
  uic_mqtt_dotdot_publish_supported_commands_IgnoreArg_unid();
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF, endpoint_id_node);
}
