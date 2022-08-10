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
  uic_mqtt_dotdot_aox_locator_attribute_position_and_orientation_valid_test_callback(
    dotdot_unid_t unid,
    dotdot_endpoint_id_t endpoint,
    bool unretained,
    uic_mqtt_dotdot_attribute_update_type_t update_type,
    bool position_and_orientation_valid)
{
  TEST_ASSERT_EQUAL_STRING("ble1334", unid);
  TEST_ASSERT_EQUAL_INT(12, endpoint);
  TEST_ASSERT_FALSE(unretained);
  TEST_ASSERT_EQUAL(UCL_REPORTED_UPDATED, update_type);
  TEST_ASSERT_TRUE(position_and_orientation_valid);
  callback_count += 1;
  return SL_STATUS_OK;
}

void test_aox_locator_receive_position_and_orientation_valid_update()
{
  // Configure a callback for the apply correction:
  uic_mqtt_dotdot_aox_locator_attribute_position_and_orientation_valid_callback_set(
    &uic_mqtt_dotdot_aox_locator_attribute_position_and_orientation_valid_test_callback);

  // Get Dotdot MQTT started
  uic_mqtt_dotdot_init();

  // Generate an incoming command published on MQTT:
  const char topic[] = "ucl/by-unid/ble1334/ep12/AoXLocator/Attributes/"
                       "PositionAndOrientationValid/Reported";

  // Test payload
  std::string payload = R"({"value" : true})";

  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL(1, callback_count);

  // Unset for following tests.
  uic_mqtt_dotdot_aox_locator_attribute_position_and_orientation_valid_callback_set(
    NULL);
}

// Test function
static sl_status_t
  uic_mqtt_dotdot_aox_locator_attribute_position_and_orientation_test_callback(
    dotdot_unid_t unid,
    dotdot_endpoint_id_t endpoint,
    bool unretained,
    uic_mqtt_dotdot_attribute_update_type_t update_type,
    CoordinateAndOrientation position)
{
  TEST_ASSERT_EQUAL_STRING("ble1334", unid);
  TEST_ASSERT_EQUAL_INT(0, endpoint);
  TEST_ASSERT_FALSE(unretained);
  TEST_ASSERT_EQUAL(UCL_DESIRED_UPDATED, update_type);
  TEST_ASSERT_EQUAL_DOUBLE(1.1, position.CoordinateX);
  TEST_ASSERT_EQUAL_DOUBLE(2.00002, position.CoordinateY);
  TEST_ASSERT_EQUAL_DOUBLE(32.23, position.CoordinateZ);
  TEST_ASSERT_EQUAL_DOUBLE(-1.1, position.OrientationX);
  TEST_ASSERT_EQUAL_DOUBLE(-2.00002, position.OrientationY);
  TEST_ASSERT_EQUAL_DOUBLE(-32.23, position.OrientationZ);
  callback_count += 1;
  return SL_STATUS_OK;
}

void test_aox_locator_receive_aox_locator_position_and_orientation_update()
{
  // Configure a callback for the apply correction:
  uic_mqtt_dotdot_aox_locator_attribute_position_and_orientation_callback_set(
    &uic_mqtt_dotdot_aox_locator_attribute_position_and_orientation_test_callback);

  // Get Dotdot MQTT started
  uic_mqtt_dotdot_init();

  // Generate an incoming command published on MQTT:
  const char topic[] = "ucl/by-unid/ble1334/ep0/AoXLocator/Attributes/"
                       "PositionAndOrientation/Desired";

  // Test payload
  std::string payload = R"(
  {
    "value": {
      "CoordinateX": 1.1,
      "CoordinateY": 2.00002,
      "CoordinateZ": 32.23,
      "OrientationX": -1.1,
      "OrientationY": -2.00002,
      "OrientationZ": -32.23
    }
  })";

  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL(1, callback_count);

  // Unset for following tests.
  uic_mqtt_dotdot_aox_locator_attribute_position_and_orientation_callback_set(
    NULL);
}

// Test function
static sl_status_t
  uic_mqtt_dotdot_aox_locator_attribute_period_samples_test_callback(
    dotdot_unid_t unid,
    dotdot_endpoint_id_t endpoint,
    bool unretained,
    uic_mqtt_dotdot_attribute_update_type_t update_type,
    int8_t period_samples)
{
  TEST_ASSERT_EQUAL_STRING("ble1334", unid);
  TEST_ASSERT_EQUAL_INT(0, endpoint);
  TEST_ASSERT_TRUE(unretained);
  TEST_ASSERT_EQUAL(UCL_REPORTED_UPDATED, update_type);
  TEST_ASSERT_EQUAL_INT8(0, period_samples);
  callback_count += 1;
  return SL_STATUS_OK;
}

void test_aox_locator_receive_aox_locator_period_sample_update_unretained()
{
  // Configure a callback for the apply correction:
  uic_mqtt_dotdot_aox_locator_attribute_period_samples_callback_set(
    &uic_mqtt_dotdot_aox_locator_attribute_period_samples_test_callback);

  // Get Dotdot MQTT started
  uic_mqtt_dotdot_init();

  // Generate an incoming command published on MQTT:
  const char topic[]
    = "ucl/by-unid/ble1334/ep0/AoXLocator/Attributes/PeriodSamples/Reported";

  // Test payload
  std::string payload = "";

  mqtt_test_helper_publish(topic, payload.c_str(), 0);
  TEST_ASSERT_EQUAL(1, callback_count);

  // Unset for following tests.
  uic_mqtt_dotdot_aox_locator_attribute_period_samples_callback_set(NULL);
}

// Test function
static sl_status_t
  uic_mqtt_dotdot_aox_locator_attribute_azimuth_mask_test_callback(
    dotdot_unid_t unid,
    dotdot_endpoint_id_t endpoint,
    bool unretained,
    uic_mqtt_dotdot_attribute_update_type_t update_type,
    size_t azimuth_mask_count,
    const MinMaxPair *azimuth_mask)
{
  TEST_ASSERT_EQUAL_STRING("ble1334", unid);
  TEST_ASSERT_EQUAL_INT(0, endpoint);
  TEST_ASSERT_FALSE(unretained);
  TEST_ASSERT_EQUAL(UCL_REPORTED_UPDATED, update_type);
  TEST_ASSERT_EQUAL(3, azimuth_mask_count);
  TEST_ASSERT_EQUAL_DOUBLE(0.1, azimuth_mask[0].Min);
  TEST_ASSERT_EQUAL_DOUBLE(0.3, azimuth_mask[0].Max);
  TEST_ASSERT_EQUAL_DOUBLE(1.1, azimuth_mask[1].Min);
  TEST_ASSERT_EQUAL_DOUBLE(1.3, azimuth_mask[1].Max);
  TEST_ASSERT_EQUAL_DOUBLE(-11.1, azimuth_mask[2].Min);
  TEST_ASSERT_EQUAL_DOUBLE(-1.3, azimuth_mask[2].Max);
  callback_count += 1;
  return SL_STATUS_OK;
}

void test_aox_locator_receive_aox_locator_azimuth_mask_update()
{
  // Configure a callback for the apply correction:
  uic_mqtt_dotdot_aox_locator_attribute_azimuth_mask_callback_set(
    &uic_mqtt_dotdot_aox_locator_attribute_azimuth_mask_test_callback);

  // Get Dotdot MQTT started
  uic_mqtt_dotdot_init();

  // Generate an incoming command published on MQTT:
  const char topic[]
    = "ucl/by-unid/ble1334/ep0/AoXLocator/Attributes/AzimuthMask/Reported";

  // Test payload
  std::string payload = R"({
  "value": [
    {
      "Min": 0.1,
      "Max": 0.3
    },
    {
      "Min": 1.1,
      "Max": 1.3
    },
    {
      "Min": -11.1,
      "Max": -1.3
    }
  ]
})";

  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL(1, callback_count);

  // Unset for following tests.
  uic_mqtt_dotdot_aox_locator_attribute_azimuth_mask_callback_set(NULL);
}

// Test function
static sl_status_t
  uic_mqtt_dotdot_aox_locator_attribute_allow_list_test_callback(
    dotdot_unid_t unid,
    dotdot_endpoint_id_t endpoint,
    bool unretained,
    uic_mqtt_dotdot_attribute_update_type_t update_type,
    size_t allow_list_count,
    const char **allow_list)
{
  TEST_ASSERT_EQUAL_STRING("ble1334", unid);
  TEST_ASSERT_EQUAL_INT(0, endpoint);
  TEST_ASSERT_FALSE(unretained);
  TEST_ASSERT_EQUAL(UCL_REPORTED_UPDATED, update_type);
  TEST_ASSERT_EQUAL(6, allow_list_count);
  TEST_ASSERT_EQUAL_STRING("Tag1", allow_list[0]);
  TEST_ASSERT_EQUAL_STRING("Tag2", allow_list[1]);
  TEST_ASSERT_EQUAL_STRING("Tag3", allow_list[2]);
  TEST_ASSERT_EQUAL_STRING("Tag4", allow_list[3]);
  TEST_ASSERT_EQUAL_STRING("Tag5", allow_list[4]);
  TEST_ASSERT_EQUAL_STRING("Tag6", allow_list[5]);
  callback_count += 1;
  return SL_STATUS_OK;
}

void test_aox_locator_receive_aox_allow_list_update()
{
  // Configure a callback for the apply correction:
  uic_mqtt_dotdot_aox_locator_attribute_allow_list_callback_set(
    &uic_mqtt_dotdot_aox_locator_attribute_allow_list_test_callback);

  // Get Dotdot MQTT started
  uic_mqtt_dotdot_init();

  // Generate an incoming command published on MQTT:
  const char topic[]
    = "ucl/by-unid/ble1334/ep0/AoXLocator/Attributes/AllowList/Reported";

  // Test payload
  std::string payload = R"({
  "value": [
    "Tag1","Tag2","Tag3","Tag4","Tag5","Tag6"
  ]
})";

  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL(1, callback_count);

  // Unset for following tests.
  uic_mqtt_dotdot_aox_locator_attribute_allow_list_callback_set(NULL);
}
}
