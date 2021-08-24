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
// Includes from this component
#include "zwave_command_class_wakeup.c"
#include "zwave_command_classes_utils.c"

// Includes from other components
#include "unity.h"
#include "attribute_store_mock.h"
#include "zpc_attribute_store_network_helper_mock.h"
#include "attribute_store_helper_mock.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute_resolver_mock.h"
#include "zwave_network_management_mock.h"
#include "zwave_command_handler_mock.h"

#include "zwave_unid_mock.h"
#include "zpc_config_mock.h"
#include "zwave_controller.h"

// Static variables
static zpc_config_t mock_config                = {};
static zwave_command_handler_t wake_up_handler = {};
static attribute_store_node_update_callback_t
  on_wake_up_version_attribute_update
  = NULL;
static attribute_store_node_t ep = 42;

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  wake_up_handler = new_command_class_handler;
  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME,
                    wake_up_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_WAKE_UP, wake_up_handler.command_class);
  TEST_ASSERT_EQUAL(3, wake_up_handler.version);
  TEST_ASSERT_NOT_NULL(wake_up_handler.control_handler);
  TEST_ASSERT_NULL(wake_up_handler.support_handler);
  TEST_ASSERT_FALSE(wake_up_handler.manual_security_validation);

  return SL_STATUS_OK;
}

static sl_status_t attribute_store_register_callback_by_type_stub(
  attribute_store_node_update_callback_t callback_function,
  attribute_store_type_t type,
  int cmock_num_calls)
{
  if (ATTRIBUTE_COMMAND_CLASS_WAKEUP_VERSION == type) {
    on_wake_up_version_attribute_update = callback_function;
  } else {
    TEST_ASSERT_TRUE_MESSAGE(
      false,
      "Received unexpected attribute_store_register_callback_by_type");
  }
  TEST_ASSERT_NOT_NULL(on_wake_up_version_attribute_update);
  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  mock_config.default_wakeup_interval = 100;
  on_wake_up_version_attribute_update = NULL;
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  attribute_store_register_callback_by_type_Stub(
    attribute_store_register_callback_by_type_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  attribute_resolver_register_rule_IgnoreAndReturn(SL_STATUS_OK);
  attribute_store_walk_tree_IgnoreAndReturn(SL_STATUS_OK);
  attribute_store_get_root_IgnoreAndReturn(1);

  zwave_command_class_wakeup_init();

  attribute_store_get_root_StopIgnore();
  attribute_store_walk_tree_StopIgnore();
  attribute_resolver_register_rule_StopIgnore();
}

static void wakeup_handler_expect()
{
  zwave_unid_from_node_id_Ignore();

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(0, 0, ep);
  attribute_store_network_helper_get_endpoint_node_IgnoreArg_endpoint_id();
  attribute_store_network_helper_get_endpoint_node_IgnoreArg_node_unid();
}

void zwave_command_class_wakeup_version_attribute_update_prototype(
  uint8_t version)
{
  attribute_store_node_t updated_node    = 0xAA;
  attribute_store_node_t endpoint_node   = 0x15;
  attribute_store_node_t my_node_id_node = 0x16;
  uint8_t value_size                     = 1;
  attribute_store_get_node_type_ExpectAndReturn(
    updated_node,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_VERSION);
  attribute_store_get_first_parent_with_type_ExpectAndReturn(
    updated_node,
    ATTRIBUTE_ENDPOINT_ID,
    endpoint_node);

  version = 1;
  attribute_store_get_node_attribute_value_ExpectAndReturn(updated_node,
                                                           REPORTED_ATTRIBUTE,
                                                           NULL,
                                                           NULL,
                                                           SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnArrayThruPtr_value(&version,
                                                                    1);
  attribute_store_get_node_attribute_value_ReturnArrayThruPtr_value_size(
    &value_size,
    1);

  if (version >= 1) {
    //Expect two attributes to be added
    attribute_store_add_if_missing_Expect(endpoint_node, 0, 2);
    attribute_store_add_if_missing_IgnoreArg_attributes();
  }
  if (version >= 2) {
    //Expect 4 attributes to be added
    attribute_store_add_if_missing_Expect(endpoint_node, 0, 4);
    attribute_store_add_if_missing_IgnoreArg_attributes();
  }
  if (version >= 3) {
    //Expect two attributes to be added
    attribute_store_add_if_missing_Expect(endpoint_node, 0, 1);
    attribute_store_add_if_missing_IgnoreArg_attributes();
  }

  if (version == 1) {
    zpc_get_config_ExpectAndReturn(&mock_config);

    attribute_store_get_first_parent_with_type_ExpectAndReturn(
      endpoint_node,
      ATTRIBUTE_NODE_ID,
      my_node_id_node);
    zwave_node_id_t my_expected_nodeid = 0x01;

    attribute_store_get_reported_ExpectAndReturn(my_node_id_node,
                                                 NULL,
                                                 sizeof(zwave_node_id_t),
                                                 SL_STATUS_OK);
    attribute_store_get_reported_IgnoreArg_value();
    attribute_store_get_reported_ReturnMemThruPtr_value(
      &my_expected_nodeid,
      sizeof(my_expected_nodeid));

    unid_t test_unid = "zw000001";
    zwave_unid_from_node_id_Expect(my_expected_nodeid, NULL);
    zwave_unid_from_node_id_IgnoreArg_unid();
    zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));
    attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
      test_unid,
      0,
      endpoint_node);
    attribute_store_node_t role_type_node = 0x88;
    attribute_store_get_node_child_by_type_ExpectAndReturn(
      endpoint_node,
      ATTRIBUTE_ZWAVE_ROLE_TYPE,
      0,
      role_type_node);
    uint8_t role_type = 0x01;
    attribute_store_get_reported_ExpectAndReturn(role_type_node,
                                                 NULL,
                                                 sizeof(uint8_t),
                                                 SL_STATUS_OK);
    attribute_store_get_reported_IgnoreArg_value();
    attribute_store_get_reported_ReturnThruPtr_value(&role_type);
    //Expect the wakeup interval set
    attribute_store_set_uint32_child_by_type_ExpectAndReturn(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_WAKEUP_INTERVAL,
      DESIRED_ATTRIBUTE,
      mock_config.default_wakeup_interval,
      SL_STATUS_OK);
  }

  zwave_network_management_get_node_id_ExpectAndReturn(1);
  zwave_network_management_is_zpc_sis_IgnoreAndReturn(true);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_NODEID,
    0,
    42);

  const zwave_node_id_t expected_nodeid = 0x01;
  attribute_store_set_desired_ExpectWithArrayAndReturn(42,
                                                       &expected_nodeid,
                                                       sizeof(expected_nodeid),
                                                       2,
                                                       SL_STATUS_OK);
  attribute_store_node_t node_id_node = 39;
  attribute_store_get_first_parent_with_type_ExpectAndReturn(updated_node,
                                                             ATTRIBUTE_NODE_ID,
                                                             node_id_node);
  attribute_resolver_set_resolution_listener_Expect(node_id_node, NULL);
  attribute_resolver_set_resolution_listener_IgnoreArg_callback();

  //Expect one attributes to be added under node_id
  attribute_store_add_if_missing_Expect(node_id_node, 0, 1);
  attribute_store_add_if_missing_IgnoreArg_attributes();

  TEST_ASSERT_NOT_NULL(on_wake_up_version_attribute_update);
  on_wake_up_version_attribute_update(updated_node, ATTRIBUTE_UPDATED);
}

void test_zwave_command_class_wakeup_version_attribute_deleted()
{
  // We expect the application to do nothing in this case.
  TEST_ASSERT_NOT_NULL(on_wake_up_version_attribute_update);
  on_wake_up_version_attribute_update(ep, ATTRIBUTE_DELETED);
}

void test_zwave_command_class_wakeup_version_attribute_update_v1()
{
  zwave_command_class_wakeup_version_attribute_update_prototype(1);
}

void test_zwave_command_class_wakeup_version_attribute_update_v2()
{
  zwave_command_class_wakeup_version_attribute_update_prototype(2);
}

void test_zwave_command_class_wakeup_version_attribute_update_v3()
{
  zwave_command_class_wakeup_version_attribute_update_prototype(3);
}

void test_zwave_command_class_wakeup_v1_interval_report()
{
  zwave_controller_connection_info_t c = {};
  const uint8_t wakeup_interval_raport_v1[]
    = {COMMAND_CLASS_WAKE_UP, WAKE_UP_INTERVAL_REPORT, 0x1, 0x2, 0x3, 0x42};

  wakeup_handler_expect();

  attribute_store_set_uint32_child_by_type_ExpectAndReturn(
    ep,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_INTERVAL,
    REPORTED_ATTRIBUTE,
    0x010203,
    SL_STATUS_OK);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    ep,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_NODEID,
    0,
    42);
  const zwave_node_id_t expected_nodeid = 0x42;
  attribute_store_set_reported_ExpectWithArrayAndReturn(42,
                                                        &expected_nodeid,
                                                        sizeof(expected_nodeid),
                                                        2,
                                                        SL_STATUS_OK);
  wake_up_handler.control_handler(&c,
                                  wakeup_interval_raport_v1,
                                  sizeof(wakeup_interval_raport_v1));
}

void test_zwave_command_class_wakeup_v2_capabilities()
{
  zwave_controller_connection_info_t c       = {};
  const uint8_t wakeup_interval_capability[] = {
    COMMAND_CLASS_WAKE_UP,
    WAKE_UP_INTERVAL_CAPABILITIES_REPORT_V2,
    0x1,
    0x2,
    0x3,

    0x11,
    0x12,
    0x13,

    0x21,
    0x22,
    0x23,

    0x31,
    0x32,
    0x33,
  };

  attribute_store_set_uint32_child_by_type_ExpectAndReturn(
    ep,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_V2_MINIMUM_INTERVAL,
    REPORTED_ATTRIBUTE,
    0x010203,
    SL_STATUS_OK);

  attribute_store_set_uint32_child_by_type_ExpectAndReturn(
    ep,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_V2_MAXIMUM_INTERVAL,
    REPORTED_ATTRIBUTE,
    0x111213,
    SL_STATUS_OK);

  attribute_store_set_uint32_child_by_type_ExpectAndReturn(
    ep,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_V2_DEFAULT_INTERVAL,
    REPORTED_ATTRIBUTE,
    0x212223,
    SL_STATUS_OK);

  attribute_store_set_uint32_child_by_type_ExpectAndReturn(
    ep,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_V2_INTERVAL_STEP,
    REPORTED_ATTRIBUTE,
    0x313233,
    SL_STATUS_OK);

  /* check that we set the default WU interval if it is not set
  */
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    ep,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_INTERVAL,
    0,
    0x66);
  attribute_store_is_value_defined_ExpectAndReturn(0x66,
                                                   REPORTED_ATTRIBUTE,
                                                   false);
  attribute_store_set_uint32_child_by_type_ExpectAndReturn(
    ep,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_INTERVAL,
    DESIRED_ATTRIBUTE,
    0x212223,
    SL_STATUS_OK);

  wakeup_handler_expect();
  wake_up_handler.control_handler(&c,
                                  wakeup_interval_capability,
                                  sizeof(wakeup_interval_capability));
}

void test_zwave_command_class_wakeup_get()
{
  uint8_t buffer[100];
  uint16_t length;
  uint8_t wui_get[] = {COMMAND_CLASS_WAKE_UP, WAKE_UP_INTERVAL_GET};
  TEST_ASSERT_EQUAL(SL_STATUS_OK, wakeup_interval_get(32, buffer, &length));
  TEST_ASSERT_EQUAL(2, length);
  TEST_ASSERT_EQUAL_INT8_ARRAY(wui_get, buffer, length);
}

void test_zwave_command_class_wakeup_set()
{
  uint16_t buffer_length = 10;
  uint8_t buffer[100];
  zwave_node_id_t nodeid = 0x42;
  uint32_t wui           = 0x010203;

  const uint8_t wui_set[]
    = {COMMAND_CLASS_WAKE_UP, WAKE_UP_INTERVAL_SET, 0x1, 0x2, 0x3, 0x42};

  attribute_store_get_node_parent_IgnoreAndReturn(0xaa);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    0xaa,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_INTERVAL,
    0,
    0x1);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    0xaa,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_NODEID,
    0,
    0x2);

  attribute_store_read_value_ExpectAndReturn(0x2,
                                             DESIRED_OR_REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(nodeid),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(&nodeid,
                                                         sizeof(nodeid));

  attribute_store_read_value_ExpectAndReturn(0x1,
                                             DESIRED_OR_REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(wui),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(&wui,
                                                         sizeof(uint32_t));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    wakeup_interval_set(32, buffer, &buffer_length));
  TEST_ASSERT_EQUAL(sizeof(wui_set), buffer_length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(wui_set, buffer, sizeof(wui_set));
}
