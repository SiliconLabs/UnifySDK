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
#include "zwave_command_class_switch_color.h"
#include "zwave_command_classes_utils.h"
#include "unity.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "zwave_unid.h"
#include "zpc_attribute_store_type_registration.h"

// Interface includes
#include "attribute_store_defined_attribute_types.h"
#include "zwave_command_class_color_switch_types.h"  //FIXME:  UIC-1901 remove this
#include "ZW_classcmd.h"
#include "zwave_utils.h"
#include "zwave_controller_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "attribute_resolver_mock.h"
#include "zpc_attribute_resolver_mock.h"
#include "zwave_command_handler_mock.h"
#include "attribute_transitions_mock.h"
#include "attribute_timeouts_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_##type

// Static variables
static zpc_resolver_event_notification_function_t on_send_complete = NULL;

// Stub functions
static sl_status_t register_send_event_handler_stub(
  attribute_store_type_t type,
  zpc_resolver_event_notification_function_t function,
  int cmock_num_calls)
{
  if (ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_STATE == type) {
    on_send_complete = function;
  }
  TEST_ASSERT_NOT_NULL(on_send_complete);
  TEST_ASSERT_EQUAL(ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_STATE, type);
  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  on_send_complete = NULL;

  datastore_init(":memory:");
  attribute_store_init();
  zpc_attribute_store_register_known_attribute_types();
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_set_home_id(home_id);
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
  is_attribute_transition_ongoing_IgnoreAndReturn(false);
}

void test_zwave_command_class_switch_color_init()
{
  // On send complete handler
  register_send_event_handler_Stub(&register_send_event_handler_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_switch_color_init());
}

void test_zwave_command_class_on_send_complete()
{
  attribute_stop_transition_IgnoreAndReturn(SL_STATUS_OK);

  // First check the frame creation.
  TEST_ASSERT_NOT_NULL(on_send_complete);

  // Create a small network with color switch values
  attribute_store_node_t state_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_STATE,
                               endpoint_id_node);

  color_component_id_t id                 = 1;
  attribute_store_node_t component_1_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_COLOR_COMPONENT_ID,
    state_node);
  attribute_store_set_reported(component_1_node, &id, sizeof(id));
  attribute_store_node_t value_1_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VALUE,
                               component_1_node);
  color_component_id_value_t value = 1;
  attribute_store_set_reported(value_1_node, &value, sizeof(value));
  value = 2;
  attribute_store_set_desired(value_1_node, &value, sizeof(value));

  attribute_store_node_t component_2_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_COLOR_COMPONENT_ID,
    state_node);
  id = 2;
  attribute_store_set_reported(component_1_node, &id, sizeof(id));
  attribute_store_node_t value_2_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VALUE,
                               component_2_node);
  value = 2;
  attribute_store_set_reported(value_2_node, &value, sizeof(value));
  value = 3;
  attribute_store_set_desired(value_2_node, &value, sizeof(value));

  on_send_complete(state_node,
                   RESOLVER_GET_RULE,
                   FRAME_SENT_EVENT_OK_SUPERVISION_WORKING);

  TEST_ASSERT_FALSE(attribute_store_is_value_matched(value_1_node));

  on_send_complete(state_node,
                   RESOLVER_SET_RULE,
                   FRAME_SENT_EVENT_OK_SUPERVISION_WORKING);

  TEST_ASSERT_FALSE(attribute_store_is_value_matched(value_1_node));

  on_send_complete(state_node,
                   RESOLVER_SET_RULE,
                   FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS);

  TEST_ASSERT_FALSE(attribute_store_is_value_matched(value_1_node));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(value_1_node, REPORTED_ATTRIBUTE));

  on_send_complete(state_node,
                   RESOLVER_SET_RULE,
                   FRAME_SENT_EVENT_OK_SUPERVISION_NO_SUPPORT);

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(value_1_node, REPORTED_ATTRIBUTE));
}
