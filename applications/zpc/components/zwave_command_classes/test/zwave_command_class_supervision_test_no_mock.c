/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include "zwave_command_class_supervision.h"
#include "zwave_command_classes_utils.h"
#include "unity.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "zpc_attribute_store_type_registration.h"

// Interface includes
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zwave_utils.h"
#include "zwave_controller_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "attribute_resolver_mock.h"
#include "zpc_attribute_resolver_mock.h"
#include "zwave_command_handler_mock.h"
#include "dotdot_mqtt_mock.h"
#include "dotdot_mqtt_generated_commands_mock.h"

// Private variables
static zwave_command_handler_t handler = {};

// Stub for registering command classes
static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SUPERVISION, handler.command_class);
  TEST_ASSERT_EQUAL(2, handler.version);
  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_NOT_NULL(handler.support_handler);
  TEST_ASSERT_FALSE(handler.manual_security_validation);

  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zpc_attribute_store_register_known_attribute_types();
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

  zwave_command_handler_register_handler_Stub(&zwave_command_handler_register_handler_stub);
  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_supervision_init());
}

/// Called after each and every test
void tearDown() {}


// We can't do it in the zwave_command_class_supervision_test since they're too many mocks
// on the attribute store side preventing us to test this feature
void test_flag_creation_on_startup_happy_case()
{
  // Flag not present
  attribute_store_node_t supervision_flag_node
    = attribute_store_get_first_child_by_type(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_SUPERVISION_ENABLED);

  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, supervision_flag_node);

  // Add it on creation
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_SUPERVISION_VERSION,
                               endpoint_id_node);

  zwave_cc_version_t version = 2;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  // Check flag node
  supervision_flag_node = attribute_store_get_first_child_by_type(
    endpoint_id_node,
    ATTRIBUTE_COMMAND_CLASS_SUPERVISION_ENABLED);

  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, supervision_flag_node);

  uint8_t flag_value = 255;

  attribute_store_get_reported(supervision_flag_node,
                               &flag_value,
                               sizeof(flag_value));
  TEST_ASSERT_EQUAL(1, flag_value);
}