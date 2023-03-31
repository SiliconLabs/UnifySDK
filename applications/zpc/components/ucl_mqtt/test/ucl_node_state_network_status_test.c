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
#include "ucl_node_state.h"
#include "unity.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"

// Interface includes
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"

// ZPC includes
#include "zpc_attribute_store_type_registration.h"
#include "zwave_utils.h"
#include "zwave_controller_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"
#include "contiki_test_helper.h"

// Mock includes
#include "dotdot_mqtt_mock.h"
#include "uic_mqtt_mock.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zpc_attribute_store_register_known_attribute_types();
  zwave_unid_set_home_id(home_id);
  contiki_test_helper_init();
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
  uic_mqtt_unretain_Ignore();
  uic_mqtt_publish_Ignore();
  uic_mqtt_dotdot_state_interview_callback_set_Ignore();
  uic_mqtt_dotdot_state_remove_offline_callback_set_Ignore();
  uic_mqtt_dotdot_state_discover_neighbors_callback_set_Ignore();
  uic_mqtt_dotdot_state_endpoint_id_list_publish_IgnoreAndReturn(SL_STATUS_OK);
  attribute_store_delete_node(attribute_store_get_root());
  zpc_attribute_store_test_helper_create_network();
  uic_mqtt_unretain_StopIgnore();
  uic_mqtt_publish_StopIgnore();
  uic_mqtt_dotdot_state_endpoint_id_list_publish_StopIgnore();
  process_start(&ucl_node_state_process, NULL);
  contiki_test_helper_run(0);
}

/// Called after each and every test
void tearDown()
{
  uic_mqtt_unretain_by_regex_Ignore();
  process_exit(&ucl_node_state_process);
  contiki_test_helper_run(0);
}

void test_ucl_node_state_network_status_test()
{
  // Don't test the State publication (publish_node_state) here.
  uic_mqtt_publish_Ignore();
  // Update the network status to OFFLINE:
  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_OFFLINE;
  // Empty commands for all clusters, but the State:
  uic_mqtt_dotdot_publish_empty_supported_commands_Expect(supporting_node_unid,
                                                          endpoint_id);
  uic_mqtt_dotdot_publish_empty_supported_commands_IgnoreArg_unid();
  uic_mqtt_dotdot_state_publish_supported_commands_Expect(supporting_node_unid,
                                                          0);
  uic_mqtt_dotdot_state_publish_supported_commands_IgnoreArg_unid();

  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));

  // Same with different values:
  network_status = NODE_STATE_TOPIC_STATE_INTERVIEW_FAIL;
  uic_mqtt_dotdot_publish_empty_supported_commands_Expect(supporting_node_unid,
                                                          endpoint_id);
  uic_mqtt_dotdot_publish_empty_supported_commands_IgnoreArg_unid();
  uic_mqtt_dotdot_state_publish_supported_commands_Expect(supporting_node_unid,
                                                          0);
  uic_mqtt_dotdot_state_publish_supported_commands_IgnoreArg_unid();

  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));

  // Same with different values:
  network_status = NODE_STATE_TOPIC_INTERVIEWING;
  uic_mqtt_dotdot_publish_empty_supported_commands_Expect(supporting_node_unid,
                                                          endpoint_id);
  uic_mqtt_dotdot_publish_empty_supported_commands_IgnoreArg_unid();
  uic_mqtt_dotdot_state_publish_supported_commands_Expect(supporting_node_unid,
                                                          0);
  uic_mqtt_dotdot_state_publish_supported_commands_IgnoreArg_unid();

  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));

  // Test when the node goes "online"
  network_status = NODE_STATE_TOPIC_STATE_INCLUDED;
  uic_mqtt_dotdot_publish_supported_commands_Expect(supporting_node_unid,
                                                    endpoint_id);
  uic_mqtt_dotdot_publish_supported_commands_IgnoreArg_unid();
  uic_mqtt_dotdot_state_publish_supported_commands_Expect(supporting_node_unid,
                                                          0);
  uic_mqtt_dotdot_state_publish_supported_commands_IgnoreArg_unid();

  uic_mqtt_dotdot_state_endpoint_id_list_publish_ExpectAndReturn(
    NULL,
    1,
    NULL,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_state_endpoint_id_list_publish_IgnoreArg_base_topic();
  uic_mqtt_dotdot_state_endpoint_id_list_publish_IgnoreArg_value();

  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));
}

void test_ucl_node_endpoint_id_deleted()
{
  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_INCLUDED;
  uic_mqtt_publish_Ignore();
  uic_mqtt_dotdot_publish_supported_commands_Expect(supporting_node_unid,
                                                    endpoint_id);
  uic_mqtt_dotdot_publish_supported_commands_IgnoreArg_unid();
  uic_mqtt_dotdot_state_publish_supported_commands_Expect(supporting_node_unid,
                                                          0);
  uic_mqtt_dotdot_state_publish_supported_commands_IgnoreArg_unid();

  uic_mqtt_dotdot_state_endpoint_id_list_publish_ExpectAndReturn(
    NULL,
    1,
    NULL,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_state_endpoint_id_list_publish_IgnoreArg_base_topic();
  uic_mqtt_dotdot_state_endpoint_id_list_publish_IgnoreArg_value();

  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));

  // Now delete the endpoint, it should trigger a publication removal and update in the EndpointIdList:
  uic_mqtt_dotdot_state_endpoint_id_list_publish_ExpectAndReturn(
    NULL,
    0,
    NULL,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_state_endpoint_id_list_publish_IgnoreArg_base_topic();
  uic_mqtt_dotdot_state_endpoint_id_list_publish_IgnoreArg_value();

  uic_mqtt_unretain_Expect("ucl/by-unid/zw-CAFECAFE-0004/ep3");
  attribute_store_delete_node(endpoint_id_node);
}

void test_cannot_retrieve_unid_values()
{
  attribute_store_node_t value_less_node_id_node
    = attribute_store_add_node(ATTRIBUTE_NODE_ID, home_id_node);

  // Add an endpoint:
  attribute_store_set_child_reported(value_less_node_id_node,
                                     ATTRIBUTE_ENDPOINT_ID,
                                     &endpoint_id,
                                     sizeof(endpoint_id));

  // Nothing should be published, we cannot find the NodeID value:
  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_INCLUDED;
  attribute_store_set_child_reported(value_less_node_id_node,
                                     ATTRIBUTE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));

  // Set also the granted keys:
  attribute_store_set_child_reported(value_less_node_id_node,
                                     ATTRIBUTE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));

  zwave_keyset_t granted_keys = 138;
  attribute_store_set_child_reported(value_less_node_id_node,
                                     ATTRIBUTE_GRANTED_SECURITY_KEYS,
                                     &granted_keys,
                                     sizeof(granted_keys));

  // finally delete the node, won't be unretained either:
  attribute_store_delete_node(value_less_node_id_node);
}
