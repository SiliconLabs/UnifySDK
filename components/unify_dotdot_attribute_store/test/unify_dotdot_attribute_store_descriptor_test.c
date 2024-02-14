/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "unify_dotdot_attribute_store.h"
#include "unify_dotdot_attribute_store_descriptor.h"
#include "unify_dotdot_defined_attribute_types.h"
#include "unity.h"

// Generic includes
#include <string.h>
#include "sl_log.h"

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"

// Mock includes
#include "dotdot_mqtt_mock.h"
#include "uic_mqtt_mock.h"

#define TEST_ENDPOINT_TYPE 3
#define TEST_NODE_TYPE     2
#define DESCRIPTOR(attribute) DOTDOT_ATTRIBUTE_ID_DESCRIPTOR_##attribute

// Test functions used for the configuration the Unify Dotdot Attribute Store Component.
static attribute_store_node_t
  get_endpoint_node(const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint_id)
{
  attribute_store_node_t node = attribute_store_get_root();
  
  node = attribute_store_get_node_child_by_value(node,
                                                 TEST_NODE_TYPE,
                                                 REPORTED_ATTRIBUTE,
                                                 (const uint8_t *)unid,
                                                 strlen(unid) + 1,
                                                 0);
  node = attribute_store_get_node_child_by_value(node,
                                                 TEST_ENDPOINT_TYPE,
                                                 REPORTED_ATTRIBUTE,
                                                 &endpoint_id,
                                                 sizeof(endpoint_id),
                                                 0);
  return node;
}

static sl_status_t get_unid(attribute_store_node_t node, char *unid)
{
  if (attribute_store_get_node_type(node) != TEST_NODE_TYPE) {
    node = attribute_store_get_first_parent_with_type(node, TEST_NODE_TYPE);
  }

  return attribute_store_get_reported_string(node, unid, MAXIMUM_UNID_SIZE);
}

static sl_status_t get_unid_endpoint(attribute_store_node_t node,
                                     char *unid,
                                     dotdot_endpoint_id_t *endpoint_id)
{
  if (attribute_store_get_node_type(node) != TEST_ENDPOINT_TYPE) {
    node = attribute_store_get_first_parent_with_type(node, TEST_ENDPOINT_TYPE);
  }
  
  if (SL_STATUS_OK
      != attribute_store_get_reported(node,
                                      endpoint_id,
                                      sizeof(dotdot_endpoint_id_t))) {
    return SL_STATUS_FAIL;
  }
  return get_unid(node, unid);
}

const unify_dotdot_attribute_store_configuration_t test_configuration = {
  .get_endpoint_node_function                  = &get_endpoint_node,
  .get_unid_endpoint_function                  = &get_unid_endpoint,
  .get_unid_function                           = &get_unid,
  .update_attribute_desired_values_on_commands = false,
  .clear_reported_on_desired_updates           = false,
  .automatic_deduction_of_supported_commands   = false,
  .force_read_attributes_enabled               = false,
  .write_attributes_enabled                    = false,
  .publish_desired_attribute_values_to_mqtt    = false,
  .publish_reported_attribute_values_to_mqtt   = false,
  .node_type                                   = TEST_NODE_TYPE,
  .endpoint_type                               = TEST_ENDPOINT_TYPE,
};

// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
}

// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

// Called before each and every test
void setUp()
{
  unify_dotdot_attribute_store_set_configuration(&test_configuration);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    unify_dotdot_attribute_store_descriptor_init());
}

// Called after each and every test
void tearDown()
{
  uic_mqtt_publish_Ignore();
  uic_mqtt_unretain_by_regex_Ignore();
}

/**
 * @brief Helper function that create a UNID/EP node in the attribute store
*/
static attribute_store_node_t create_node_ep(char *unid,
                                             dotdot_endpoint_id_t endpoint_id)
{
  attribute_store_node_t unid_node
    = attribute_store_add_node(TEST_NODE_TYPE, attribute_store_get_root());
  attribute_store_node_t endpoint_node
    = attribute_store_add_node(TEST_ENDPOINT_TYPE, unid_node);
  
  attribute_store_set_reported_string(unid_node, unid);
  attribute_store_set_reported(endpoint_node,
                               &endpoint_id,
                               sizeof(endpoint_id));

  return endpoint_node;
}


void test_publish_device_type_list()
{
  // Create an unid/endpoint:
  char unid_value[]                = "unid-42";
  dotdot_endpoint_id_t endpoint_id = 4;
  attribute_store_node_t endpoint_node
    = create_node_ep(unid_value, endpoint_id);
  
  uic_mqtt_publish_Expect(
    "ucl/by-unid/unid-42/ep4/Descriptor/Attributes/DeviceTypeList/Desired",
    "{\"value\":[{\"DeviceType\":\"DoorLock\",\"Revision\":1}]}",
     strlen("{\"value\":[{\"DeviceType\":\"DoorLock\",\"Revision\":1}]}"),
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/unid-42/ep4/Descriptor/Attributes/DeviceTypeList/Reported",
    "{\"value\":[{\"DeviceType\":\"DoorLock\",\"Revision\":1}]}",
     strlen("{\"value\":[{\"DeviceType\":\"DoorLock\",\"Revision\":1}]}"),
    true);
  DeviceTypeStruct device_type = {ZCL_DEV_TYPE_ID_DOOR_LOCK, 1};
  attribute_store_emplace(endpoint_node,
                          DESCRIPTOR(DEVICE_TYPE_LIST),
                          &device_type,
                          sizeof(device_type));
  
  uic_mqtt_publish_Expect(
    "ucl/by-unid/unid-42/ep4/Descriptor/Attributes/DeviceTypeList/Desired",
    "{\"value\":[{\"DeviceType\":\"DoorLockController\",\"Revision\":2}]}",
    strlen("{\"value\":[{\"DeviceType\":\"DoorLockController\",\"Revision\":2}]}"),
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/unid-42/ep4/Descriptor/Attributes/DeviceTypeList/Reported",
    "{\"value\":[{\"DeviceType\":\"DoorLockController\",\"Revision\":2}]}",
     strlen("{\"value\":[{\"DeviceType\":\"DoorLockController\",\"Revision\":2}]}"),
    true);
  DeviceTypeStruct device_type_new = {ZCL_DEV_TYPE_ID_DOOR_LOCK_CONTROLLER, 2};
  attribute_store_set_child_reported(endpoint_node,
                                    DESCRIPTOR(DEVICE_TYPE_LIST),
                                    &device_type_new,
                                    sizeof(device_type_new));
}
