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

#include "keep_sleeping_nodes_alive.h"

attribute_store_node_changed_callback_t *frame_received_cb = NULL;
attribute_store_node_changed_callback_t *network_status_cb = NULL;

void keep_alive_received_frame(attribute_store_node_t node,
                               attribute_store_change_t change);
void keep_alive_network_status_update(attribute_store_node_t node,
                                      attribute_store_change_t change);
void keep_alive_nop_node(void *user);
#include "attribute.hpp"

extern "C" {
#include "attribute_store_defined_attribute_types.h"
#include "zwave_node_id_definitions.h"
#include "zwave_generic_types.h"
#include "ucl_definitions.h"
#include "contiki_test_helper.h"
#include "zwave_controller_utils_mock.h"
#include "zpc_attribute_store_test_helper.h"

// Unify includes
#include "datastore.h"
#include "attribute_store_fixt.h"
using attribute_store::attribute;

void suiteSetUp()
{
  contiki_test_helper_init();
  contiki_test_helper_run_once(0);
  datastore_init(":memory:");
  attribute_store_init();
}

int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

void setUp()
{
  initialize_keep_alive_for_sleeping_nodes();
}

static void init_network(attribute &node_id_node, bool set_op_mode = true)
{
  zpc_attribute_store_test_helper_create_network();

  attribute::root().visit([&node_id_node](attribute &a, int depth) {
    if (a.type() == ATTRIBUTE_NODE_ID) {
      node_id_node = a;
      return SL_STATUS_ABORT;
    }
    return SL_STATUS_OK;
  });

  attribute last_received_node
    = node_id_node.add_node(ATTRIBUTE_LAST_RECEIVED_FRAME_TIMESTAMP);
  unsigned long current_time = clock_seconds();
  last_received_node.set_reported(current_time);

  if (set_op_mode) {
    uint8_t listening_protocol = 0;
    node_id_node.add_node(ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING)
      .set_reported(listening_protocol);

    uint8_t optional_protocol = 0;
    node_id_node.add_node(ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL)
      .set_reported(optional_protocol);
  }

  node_state_topic_state_t state = NODE_STATE_TOPIC_INTERVIEWING;
  attribute network_status = node_id_node.add_node(ATTRIBUTE_NETWORK_STATUS);
  network_status.set_reported(state);
}

void test_create_network_status_no_parent()
{
  attribute node_id_node;
  init_network(node_id_node);

  // test 1: dont expect anything when time passed under the timeout
  contiki_test_helper_run(3000);

  zwave_send_nop_to_node_ExpectAndReturn(
    node_id_node.reported<zwave_node_id_t>(),
    ZWAVE_TX_QOS_RECOMMENDED_TIMING_CRITICAL_PRIORITY,
    4000,
    nullptr,
    nullptr,
    SL_STATUS_OK);

  // test2: test nop is send after time out
  contiki_test_helper_run(2000);

  zwave_send_nop_to_node_ExpectAndReturn(
    node_id_node.reported<zwave_node_id_t>(),
    ZWAVE_TX_QOS_RECOMMENDED_TIMING_CRITICAL_PRIORITY,
    4000,
    nullptr,
    nullptr,
    SL_STATUS_OK);

  // test3: last received frame prevents node from being removed from the list
  attribute last_received_node
    = node_id_node.child_by_type(ATTRIBUTE_LAST_RECEIVED_FRAME_TIMESTAMP);
  last_received_node.set_reported(10);
  contiki_test_helper_run(10000);

  //test4: remove by timeout
  contiki_test_helper_run(25000);
}

void test_attribute_deleted()
{
  attribute node_id_node;
  init_network(node_id_node);

  node_id_node.delete_node();
  //  dont expect anything node is deleted
  contiki_test_helper_run(5000);
}

void test_attribute_deleted_on_state_change()
{
  attribute node_id_node;
  init_network(node_id_node);

  attribute network_status
    = node_id_node.child_by_type(ATTRIBUTE_NETWORK_STATUS);
  node_state_topic_state_t state = NODE_STATE_TOPIC_STATE_OFFLINE;

  network_status.set_reported(state);

  //  dont expect anything
  contiki_test_helper_run(5000);
}

void test_nodes_that_are_not_wake_up_are_ignored()
{
  attribute node_id_node;
  init_network(node_id_node, false);

  attribute network_status
    = node_id_node.child_by_type(ATTRIBUTE_NETWORK_STATUS);
  node_state_topic_state_t state = NODE_STATE_TOPIC_INTERVIEWING;

  network_status.set_reported(state);

  //  dont expect anything
  contiki_test_helper_run(5000);
}
}