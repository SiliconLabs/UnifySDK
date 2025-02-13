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

#include <stdlib.h>

#include "workaround.hpp"
#include "unity.h"
#include "unity_helpers.h"
#include "cmock.h"

#include "sl_status.h"

#include "mqtt_test_helper.h"

extern "C" {
#include "dotdot_mqtt.h"
#include "dotdot_mqtt_attributes.h"

// Static variables
static int callback_count = 0;

/// Called before each and every test
void setUp()
{
  callback_count = 0;
  mqtt_test_helper_init();
}

// Test function
static sl_status_t
  uic_mqtt_dotdot_on_off_attribute_on_off_test_callback(
    dotdot_unid_t unid,
    dotdot_endpoint_id_t endpoint,
    bool unretained,
    uic_mqtt_dotdot_attribute_update_type_t update_type,
    bool on_off)
{
  TEST_ASSERT_EQUAL_STRING("zw-cafecafe-1010", unid);
  TEST_ASSERT_EQUAL_INT(12, endpoint);
  TEST_ASSERT_FALSE(unretained);
  TEST_ASSERT_EQUAL(UCL_REPORTED_UPDATED, update_type);
  TEST_ASSERT_TRUE(on_off);
  callback_count += 1;
  return SL_STATUS_OK;
}

void test_on_off_on_off_update()
{
  // Configure a callback for the apply correction:
  uic_mqtt_dotdot_on_off_attribute_on_off_callback_set(
    &uic_mqtt_dotdot_on_off_attribute_on_off_test_callback);

  // Get Dotdot MQTT started
  uic_mqtt_dotdot_init();

  // Generate an incoming command published on MQTT:
  const char topic[] = "ucl/by-unid/zw-cafecafe-1010/ep12/OnOff/Attributes/"
                       "OnOff/Reported";

  // Test payload
  std::string payload = R"({"value" : true})";

  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL(1, callback_count);

  // Unset for following tests.
  uic_mqtt_dotdot_on_off_attribute_on_off_callback_set(
    NULL);
}

// Test function
static sl_status_t
  uic_mqtt_dotdot_on_off_attribute_on_time_test_callback(
    dotdot_unid_t unid,
    dotdot_endpoint_id_t endpoint,
    bool unretained,
    uic_mqtt_dotdot_attribute_update_type_t update_type,
    uint16_t on_time)
{
  TEST_ASSERT_EQUAL_STRING("zw-cafecafe-1010", unid);
  TEST_ASSERT_EQUAL_INT(0, endpoint);
  TEST_ASSERT_TRUE(unretained);
  TEST_ASSERT_EQUAL(UCL_REPORTED_UPDATED, update_type);
  TEST_ASSERT_EQUAL_INT8(0, on_time);
  callback_count += 1;
  return SL_STATUS_OK;
}

void test_on_off_on_time_update_unretained()
{
  // Configure a callback for the apply correction:
  uic_mqtt_dotdot_on_off_attribute_on_time_callback_set(
    &uic_mqtt_dotdot_on_off_attribute_on_time_test_callback);

  // Get Dotdot MQTT started
  uic_mqtt_dotdot_init();

  // Generate an incoming command published on MQTT:
  const char topic[]
    = "ucl/by-unid/zw-cafecafe-1010/ep0/OnOff/Attributes/OnTime/Reported";

  // Test payload
  std::string payload = "";

  mqtt_test_helper_publish(topic, payload.c_str(), 0);
  TEST_ASSERT_EQUAL(1, callback_count);

  // Unset for following tests.
  uic_mqtt_dotdot_on_off_attribute_on_time_callback_set(NULL);
}

// Test function
static sl_status_t
  uic_mqtt_dotdot_descriptor_attribute_device_type_list_test_callback(
    dotdot_unid_t unid,
    dotdot_endpoint_id_t endpoint,
    bool unretained,
    uic_mqtt_dotdot_attribute_update_type_t update_type,
    size_t device_type_list_count,
    const DeviceTypeStruct* device_type_list)
{
  TEST_ASSERT_EQUAL_STRING("zw-cafecafe-1010", unid);
  TEST_ASSERT_EQUAL_INT(0, endpoint);
  TEST_ASSERT_FALSE(unretained);
  TEST_ASSERT_EQUAL(UCL_REPORTED_UPDATED, update_type);
  TEST_ASSERT_EQUAL(2, device_type_list_count);
  TEST_ASSERT_EQUAL_INT(257, device_type_list[0].DeviceType);
  TEST_ASSERT_EQUAL_INT(1, device_type_list[0].Revision);
  TEST_ASSERT_EQUAL_INT(268, device_type_list[1].DeviceType);
  TEST_ASSERT_EQUAL_INT(2, device_type_list[1].Revision);
  callback_count += 1;
  return SL_STATUS_OK;
}

void test_descriptor_device_type_list_update()
{
  // Configure a callback for the apply correction:
  uic_mqtt_dotdot_descriptor_attribute_device_type_list_callback_set(
    &uic_mqtt_dotdot_descriptor_attribute_device_type_list_test_callback);

  // Get Dotdot MQTT started
  uic_mqtt_dotdot_init();

  // Generate an incoming command published on MQTT:
  const char topic[]
    = "ucl/by-unid/zw-cafecafe-1010/ep0/Descriptor/Attributes/DeviceTypeList/Reported";

  // Test payload
  std::string payload = R"({
  "value": [
    {
      "DeviceType": 257,
      "Revision": 1
    },
    {
      "DeviceType": 268,
      "Revision": 2
    }
  ]
})";

  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL(1, callback_count);

  // Unset for following tests.
  uic_mqtt_dotdot_descriptor_attribute_device_type_list_callback_set(NULL);
}

// Test function
static sl_status_t
  uic_mqtt_dotdot_binding_attribute_bindale_cluster_list_test_callback(
    dotdot_unid_t unid,
    dotdot_endpoint_id_t endpoint,
    bool unretained,
    uic_mqtt_dotdot_attribute_update_type_t update_type,
    size_t bindable_cluster_list_count,
    const char **bindable_cluster_list)
{
  TEST_ASSERT_EQUAL_STRING("zw-cafecafe-1010", unid);
  TEST_ASSERT_EQUAL_INT(0, endpoint);
  TEST_ASSERT_FALSE(unretained);
  TEST_ASSERT_EQUAL(UCL_REPORTED_UPDATED, update_type);
  TEST_ASSERT_EQUAL(6, bindable_cluster_list_count);
  TEST_ASSERT_EQUAL_STRING("Tag1", bindable_cluster_list[0]);
  TEST_ASSERT_EQUAL_STRING("Tag2", bindable_cluster_list[1]);
  TEST_ASSERT_EQUAL_STRING("Tag3", bindable_cluster_list[2]);
  TEST_ASSERT_EQUAL_STRING("Tag4", bindable_cluster_list[3]);
  TEST_ASSERT_EQUAL_STRING("Tag5", bindable_cluster_list[4]);
  TEST_ASSERT_EQUAL_STRING("Tag6", bindable_cluster_list[5]);
  callback_count += 1;
  return SL_STATUS_OK;
}

void test_binding_bindable_cluster_list_update()
{
  // Configure a callback for the apply correction:
  uic_mqtt_dotdot_binding_attribute_bindable_cluster_list_callback_set(
    &uic_mqtt_dotdot_binding_attribute_bindale_cluster_list_test_callback);

  // Get Dotdot MQTT started
  uic_mqtt_dotdot_init();

  // Generate an incoming command published on MQTT:
  const char topic[]
    = "ucl/by-unid/zw-cafecafe-1010/ep0/Binding/Attributes/BindableClusterList/Reported";

  // Test payload
  std::string payload = R"({
  "value": [
    "Tag1","Tag2","Tag3","Tag4","Tag5","Tag6"
  ]
})";

  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL(1, callback_count);

  // Unset for following tests.
  uic_mqtt_dotdot_binding_attribute_bindable_cluster_list_callback_set(NULL);
}
}
