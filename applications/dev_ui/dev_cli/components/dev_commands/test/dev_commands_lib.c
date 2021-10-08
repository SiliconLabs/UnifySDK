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

#include "unity.h"
#include "command_smartstartlist_lib.hpp"

// Setup the test suite
void suiteSetUp()
{
  printf("Initialising test");
}

// Test parsing SmartStart list in correct format
void test_parsing_smart_start_list_correct_format()
{
  const char *ss_list
    = "["
      "{ \"DSK\": \"24859-64107-46202-12845-60475-62452-54892-59867\","
      "\"Include\": true,"
      "\"ProtocolControllerUnid\": \"98408564651607\","
      "\"Unid\": \"\""
      "},"
      "{ \"DSK\": \"29304-00703-03201-39471-03987-12013-63902-39874\","
      "\"Include\" : false,"
      "\"ProtocolControllerUnid\" : \"\","
      "\"Unid\" : \"\""
      "}"
      "]";

  int state = parse_json_smart_start_list(ss_list);
  TEST_ASSERT_EQUAL(state, 0);
}

// Test parsing SmartStart list in wrong format
void test_parsing_smart_start_list_wrong_format()
{
  const char *ss_list
    = ""  // Error here
      "{ \"DSK\": \"24859-64107-46202-12845-60475-62452-54892-59867\","
      "\"Include\": true,"
      "\"ProtocolControllerUnid\": \"98408564651607\","
      "\"Unid\": \"\""
      ""  // Error here
      "{ \"DSK\": \"29304-00703-03201-39471-03987-12013-63902-39874\","
      "\"Include\" : false,"
      "\"ProtocolControllerUnid\" : \"\""  // Error here
      "\"Unid\" : \"\""
      "}"
      "]";

  int state = parse_json_smart_start_list(ss_list);
  TEST_ASSERT_EQUAL(state, 1);
}

void test_removing_an_empty_dsk_key()
{
  // It will fail since the DSK key is empty
  // Testing succes relies on a MQTT broker so it is not tested
  int state = remove_entry_smart_start_list("");
  TEST_ASSERT_EQUAL(state, 1);
}

void test_adding_an_empty_dsk_key()
{
  // It will fail with an empty DSK key
  // Testing sucess relies on a MQTT broker so is not tested
  int state = update_smart_start_list("", "", "", "");
  TEST_ASSERT_EQUAL(state, 1);
}
/// Teardown the test suite
int suiteTearDown(int num_failures)
{
  printf("Ending test suite");
  return num_failures;
}