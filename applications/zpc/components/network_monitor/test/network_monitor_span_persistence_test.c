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
#include "network_monitor_span_persistence.h"
#include "unity.h"

// UIC Components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_fixt.h"
#include "attribute_store_helper.h"

// ZPC Components
#include "attribute_store_defined_attribute_types.h"

// Helper includes
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "zwave_network_management_mock.h"
#include "zwave_s2_nonce_management_mock.h"
#include "zwave_tx_groups_mock.h"

// Generic includes
#include <string.h>

// Static test variables
static zwave_nodemask_t current_node_list = {};
static zwave_nodemask_t group_node_list   = {};
static span_entry_t test_span_data        = {};
static mpan_entry_t test_mpan_data        = {};

// Mock of the network monitor functions itself
uint8_t *network_monitor_get_cached_current_node_list()
{
  return current_node_list;
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

  // Set out current network mask correctly
  memset(current_node_list, 0, sizeof(zwave_nodemask_t));
  ZW_ADD_NODE_TO_MASK(zpc_node_id, current_node_list);
  ZW_ADD_NODE_TO_MASK(node_id, current_node_list);

  // Reset group content
  memset(group_node_list, 0, sizeof(zwave_nodemask_t));
}

void test_back_up_and_restore_span_data()
{
  // We have 1 node in our network. We will ask S2 to give us its span data
  test_span_data.df               = 23;
  test_span_data.rx_sequence      = 20;
  test_span_data.class_id         = 3;
  test_span_data.tx_sequence      = 3;
  test_span_data.working_state[4] = 234;
  test_span_data.key[10]          = 38;

  zwave_s2_get_span_data_ExpectAndReturn(node_id, NULL, SL_STATUS_OK);
  zwave_s2_get_span_data_IgnoreArg_span_data();
  zwave_s2_get_span_data_ReturnMemThruPtr_span_data(&test_span_data,
                                                    sizeof(test_span_data));

  network_monitor_store_span_table_data();

  // SPAN entry created under the NodeID
  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(node_id_node));
  attribute_store_node_t span_entry_node
    = attribute_store_get_node_child_by_value(node_id_node,
                                              ATTRIBUTE_S2_SPAN_ENTRY,
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&test_span_data,
                                              sizeof(test_span_data),
                                              0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, span_entry_node);

  // Now that we have an entry, verify that it gets loaded on restore
  zwave_s2_set_span_table_ExpectWithArray(node_id,
                                          &test_span_data,
                                          sizeof(test_span_data));

  network_monitor_restore_span_table_data();
}

void test_back_up_span_data_not_available()
{
  zwave_s2_get_span_data_IgnoreAndReturn(SL_STATUS_NOT_FOUND);
  network_monitor_store_span_table_data();

  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(node_id_node));
}

void test_restore_span_data_nothing_available()
{
  // Nothing should happen here, nothing to restore from the attribute store.
  network_monitor_restore_span_table_data();
}

void test_back_up_and_restore_mpan_data()
{
  // We have 1 node in our network. We will ask S2 to give us its span data
  test_mpan_data.inner_state[0] = 23;
  test_mpan_data.inner_state[1] = 24;
  test_mpan_data.inner_state[9] = 0x9D;
  test_mpan_data.group_id       = 1;
  test_mpan_data.class_id       = 9;
  test_mpan_data.owner_node_id  = 24;

  ZW_ADD_NODE_TO_MASK(node_id, group_node_list);

  // Let's say NodeID 4 is part of Group 2 and 24

  for (zwave_multicast_group_id_t group_id = 1; group_id < 255; group_id++) {
    if (group_id == 2 || group_id == 24) {
      zwave_s2_get_mpan_data_ExpectAndReturn(0, group_id, NULL, SL_STATUS_OK);
      zwave_s2_get_mpan_data_IgnoreArg_mpan_data();
      zwave_s2_get_mpan_data_ReturnMemThruPtr_mpan_data(&test_mpan_data,
                                                        sizeof(test_mpan_data));

      // Then the list of NodeIDs for the group:
      zwave_tx_get_nodes_ExpectAndReturn(NULL, group_id, SL_STATUS_OK);
      zwave_tx_get_nodes_IgnoreArg_nodes();
      zwave_tx_get_nodes_ReturnMemThruPtr_nodes(group_node_list,
                                                sizeof(group_node_list));

    } else {
      // Not part of the group in the first place
      zwave_s2_get_mpan_data_ExpectAndReturn(0,
                                             group_id,
                                             NULL,
                                             SL_STATUS_NOT_FOUND);
      zwave_s2_get_mpan_data_IgnoreArg_mpan_data();
    }
  }

  network_monitor_store_mpan_table_data();
  attribute_store_log();

  // MPAN table created under the ZPC NodeID
  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(zpc_node_id_node));
  attribute_store_node_t mpan_table_node
    = attribute_store_get_node_child_by_type(zpc_node_id_node,
                                             ATTRIBUTE_S2_MPAN_TABLE,
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, mpan_table_node);

  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(mpan_table_node));
  TEST_ASSERT_NOT_EQUAL(
    ATTRIBUTE_STORE_INVALID_NODE,
    attribute_store_get_node_child_by_value(mpan_table_node,
                                            ATTRIBUTE_S2_MPAN_ENTRY,
                                            REPORTED_ATTRIBUTE,
                                            (uint8_t *)&test_mpan_data,
                                            sizeof(test_mpan_data),
                                            0));

  TEST_ASSERT_NOT_EQUAL(
    ATTRIBUTE_STORE_INVALID_NODE,
    attribute_store_get_node_child_by_value(mpan_table_node,
                                            ATTRIBUTE_S2_MPAN_ENTRY,
                                            REPORTED_ATTRIBUTE,
                                            (uint8_t *)&test_mpan_data,
                                            sizeof(test_mpan_data),
                                            1));

  // Group table created under the NodeID.
  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(node_id_node));
  attribute_store_node_t group_list_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_MULTICAST_GROUP_LIST,
                                             0);
  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(group_list_node));
  zwave_multicast_group_id_t group_id = 2;
  TEST_ASSERT_NOT_EQUAL(
    ATTRIBUTE_STORE_INVALID_NODE,
    attribute_store_get_node_child_by_value(group_list_node,
                                            ATTRIBUTE_MULTICAST_GROUP,
                                            REPORTED_ATTRIBUTE,
                                            (uint8_t *)&group_id,
                                            sizeof(group_id),
                                            0));
  group_id = 24;
  TEST_ASSERT_NOT_EQUAL(
    ATTRIBUTE_STORE_INVALID_NODE,
    attribute_store_get_node_child_by_value(group_list_node,
                                            ATTRIBUTE_MULTICAST_GROUP,
                                            REPORTED_ATTRIBUTE,
                                            (uint8_t *)&group_id,
                                            sizeof(group_id),
                                            0));

  // Now that we have an entry, verify that it gets loaded on restore
  zwave_s2_set_mpan_data_ExpectWithArray(0,
                                         test_mpan_data.group_id,
                                         &test_mpan_data,
                                         sizeof(test_mpan_data));
  zwave_s2_set_mpan_data_ExpectWithArray(0,
                                         test_mpan_data.group_id,
                                         &test_mpan_data,
                                         sizeof(test_mpan_data));
  zwave_tx_add_node_to_group_ExpectAndReturn(node_id, 2, SL_STATUS_OK);
  zwave_tx_add_node_to_group_ExpectAndReturn(node_id, 24, SL_STATUS_FAIL);

  network_monitor_restore_mpan_table_data();
}

void test_back_up_mpan_data_not_available()
{
  zwave_s2_get_mpan_data_IgnoreAndReturn(SL_STATUS_NOT_FOUND);
  network_monitor_store_mpan_table_data();

  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(node_id_node));
  // Empty MPAN table created under the ZPC node.
  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(zpc_node_id_node));
}

void test_restore_mpan_data_nothing_available()
{
  // Nothing should happen here, nothing to restore from the attribute store.
  network_monitor_restore_mpan_table_data();
}
