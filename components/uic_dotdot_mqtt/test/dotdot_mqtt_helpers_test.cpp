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
#include "workaround.hpp"
#include "dotdot_mqtt_helpers.h"

extern "C" {
#include "unity.h"
/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_unid_from_topic()
{
  TEST_ASSERT_EQUAL_STRING(
    "1234horse",
    unify::unid_from_mqtt_topic("ucl/by-unid/1234horse/State").value().c_str());
  TEST_ASSERT_EQUAL_STRING(
    "42",
    unify::unid_from_mqtt_topic(
      std::string("ucl/by-unid/42/ep0/SupportedCommands"))
      .value()
      .c_str());
  TEST_ASSERT_FALSE(
    unify::unid_from_mqtt_topic("ucl/by-group/42/OnOff/Commands/Off")
      .has_value());
}

void test_unid_from_topic_c()
{
  char result[100];
  TEST_ASSERT_EQUAL_STRING("1234horse",
                           unid_from_mqtt_topic_c("ucl/by-unid/1234horse/State",
                                                  result,
                                                  sizeof(result)));
  TEST_ASSERT_EQUAL_STRING(
    "42",
    unid_from_mqtt_topic_c("ucl/by-unid/42/ep0/SupportedCommands",
                           result,
                           sizeof(result)));
  TEST_ASSERT_NULL(unid_from_mqtt_topic_c("ucl/by-group/42/OnOff/Commands/Off",
                                          result,
                                          sizeof(result)));
}

void test_enum_from_string()
{
  TEST_ASSERT_EQUAL(
    1,
    node_state_network_status_get_enum_value_number("Online interviewing"));
  TEST_ASSERT_EQUAL(
    3,
    node_state_network_status_get_enum_value_number("Unavailable"));
}

void test_enum_from_string_c()
{
  TEST_ASSERT_EQUAL(
    1,
    node_state_network_status_get_enum_value_number_c("Online interviewing"));
  TEST_ASSERT_EQUAL(
    3,
    node_state_network_status_get_enum_value_number_c("Unavailable"));
}

void test_string_from_enum()
{
  TEST_ASSERT_EQUAL_STRING(
    "Online functional",
    node_state_network_status_get_enum_value_name(0).c_str());
  TEST_ASSERT_EQUAL_STRING(
    "Offline",
    node_state_network_status_get_enum_value_name(4).c_str());

  TEST_ASSERT_EQUAL_STRING(
    "42",
    node_state_network_status_get_enum_value_name(42).c_str());
}

void test_string_from_enum_c()
{
  char result[100];
  TEST_ASSERT_EQUAL_STRING(
    "Online functional",
    node_state_network_status_get_enum_value_name_c(0, result, sizeof(result)));
  TEST_ASSERT_EQUAL_STRING(
    "Offline",
    node_state_network_status_get_enum_value_name_c(4, result, sizeof(result)));

  // In case node_state_network_status_get_enum_value_name_c fails it will
  // return the string representation of the value
  TEST_ASSERT_EQUAL_STRING(
    "10",
    node_state_network_status_get_enum_value_name_c(10,
                                                    result,
                                                    sizeof(result)));
}
}
