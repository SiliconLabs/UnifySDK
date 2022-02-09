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
#include "workaround.hpp"
#include "smartstart_management.hpp"
#include "unity.h"
#include "contiki_test_helper.h"

#define LOG_TAG "smartstart_management_test"

const std::string MY_UNID        = "zw-BEEFFEEB-001";
const std::string DEVICE_15_UNID = "zw-BEEFFEEB-015";
const std::string DEVICE_16_UNID = "zw-BEEFFEEB-016";

#define SMARTSTART_LIST_TOPIC        "ucl/SmartStart/List"
#define SMARTSTART_LIST_UPDATE_TOPIC "ucl/SmartStart/List/Update"

const std::vector<smartstart::Entry> test_entries_with_mostly_true
  = {smartstart::Entry(
       "24859-64107-46202-12845-60475-62452-54892-59867", true, MY_UNID, ""),
     smartstart::Entry("24859-12345-46202-12845-60475-62452-54892-59867",
                       true,
                       MY_UNID,
                       DEVICE_15_UNID),
     smartstart::Entry(
       "24859-78787-46202-12845-60475-62452-54892-59867", true, "", ""),
     smartstart::Entry(
       "12345-67895-46202-12845-60475-62452-54892-59867", true, "", ""),
     smartstart::Entry(
       "12345-64107-46202-12845-60475-62452-54892-59867", false, "", ""),
     smartstart::Entry(
       "55555-64107-46202-12845-60475-62452-54892-59867", true, "", ""),
     smartstart::Entry(
       "22222-64107-46202-12845-60475-62452-54892-59867", true, "", ""),
     smartstart::Entry(
       "20212-64107-46202-12845-60475-62452-54892-59867", true, "", ""),
     smartstart::Entry(
       "33333-64107-46202-12845-60475-62452-54892-59867", true, "", ""),
     smartstart::Entry(
       "11111-64107-46202-12845-60475-62452-54892-59867", true, "", "")};

const std::vector<smartstart::Entry> test_entries_with_false
  = {smartstart::Entry(
       "24859-64107-46202-12845-60475-62452-54892-59867", false, "", ""),
     smartstart::Entry(
       "24859-12345-46202-12845-60475-62452-54892-59867", false, "", ""),
     smartstart::Entry(
       "24859-78787-46202-12845-60475-62452-54892-59867", false, "", ""),
     smartstart::Entry(
       "12345-67895-46202-12845-60475-62452-54892-59867", false, "", ""),
     smartstart::Entry(
       "12345-64107-46202-12845-60475-62452-54892-59867", false, "", ""),
     smartstart::Entry(
       "55555-64107-46202-12845-60475-62452-54892-59867", false, "", ""),
     smartstart::Entry(
       "22222-64107-46202-12845-60475-62452-54892-59867", false, "", ""),
     smartstart::Entry(
       "20212-64107-46202-12845-60475-62452-54892-59867", false, "", ""),
     smartstart::Entry(
       "33333-64107-46202-12845-60475-62452-54892-59867", false, "", ""),
     smartstart::Entry(
       "11111-64107-46202-12845-60475-62452-54892-59867", false, "", "")};

const std::string partial_dsk_to_get_all_entries = "59867";

std::string
  generate_smartstart_list(const std::vector<smartstart::Entry> entries)
{
  std::string smartstart_list = "{\"value\": ";
  smartstart_list += "[";
  bool comma = false;
  for (const smartstart::Entry &e: entries) {
    if (comma) {
      smartstart_list += ",";
    } else {
      comma = true;
    }
    smartstart_list += "{";
    smartstart_list += "\"DSK\":\"" + e.dsk + "\",";
    smartstart_list
      += "\"Include\":" + std::string(e.include ? "true" : "false") + ",";
    smartstart_list
      += "\"ProtocolControllerUnid\":\"" + e.protocol_controller_unid + "\",";
    smartstart_list += "\"Unid\":\"" + e.device_unid + "\"";
    smartstart_list += "}";
  }
  smartstart_list += "]}";
  return smartstart_list;
}

extern "C" {

#include "uic_mqtt_mock.h"

typedef void (*mqtt_on_list_update_callback)(const char *topic,
                                             const char *message,
                                             const size_t message_length);
mqtt_on_list_update_callback my_mqtt_on_list_update;
static void mqtt_on_list_update_callback_save(
  const char *topic, mqtt_on_list_update_callback callback, int n)
{
  my_mqtt_on_list_update = callback;
}
static bool smartstart_enable = false;
static void on_entries_awaiting_inclusion(bool value)
{
  smartstart_enable = value;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp()
{
  contiki_test_helper_init();
  smartstart_enable = false;
  uic_mqtt_subscribe_AddCallback(mqtt_on_list_update_callback_save);
  uic_mqtt_subscribe_Expect("", NULL);
  uic_mqtt_subscribe_IgnoreArg_topic();
  uic_mqtt_subscribe_IgnoreArg_callback();
  smartstart::Management::get_instance()->init(MY_UNID,
                                               &on_entries_awaiting_inclusion);
}
void tearDown()
{
  smartstart::Management::get_instance()->teardown();
}

/// Test all possible queries on SmartStart list formed mostly by true entries
void test_query_on_mostly_true_entries()
{
  std::string smartstart_list
    = generate_smartstart_list(test_entries_with_mostly_true);
  my_mqtt_on_list_update("", smartstart_list.c_str(), smartstart_list.size());
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(smartstart_enable, true);

  /// Partial DSK
  smartstart::Query q(smartstart::QueryType::partial,
                      smartstart::QueryKey::dsk,
                      partial_dsk_to_get_all_entries);
  std::vector<smartstart::Entry> query_result
    = smartstart::Management::get_instance()->get_cache_entries(q);
  TEST_ASSERT_EQUAL_INT(query_result.size(),
                        test_entries_with_mostly_true.size());
  TEST_ASSERT_EQUAL(
    smartstart::Management::get_instance()->has_entries_awaiting_inclusion(),
    true);

  /// Exact DSK
  smartstart::Query q2
    = smartstart::Query(smartstart::QueryType::exact,
                        smartstart::QueryKey::dsk,
                        test_entries_with_mostly_true[0].dsk);
  std::vector<smartstart::Entry> query_result2
    = smartstart::Management::get_instance()->get_cache_entries(q2);
  TEST_ASSERT_EQUAL_INT(query_result2.size(), 1);

  /// Exact include
  smartstart::Query q3 = smartstart::Query(smartstart::QueryType::exact,
                                           smartstart::QueryKey::include,
                                           true);
  std::vector<smartstart::Entry> query_result3
    = smartstart::Management::get_instance()->get_cache_entries(q3);
  TEST_ASSERT_EQUAL_INT(query_result3.size(),
                        test_entries_with_mostly_true.size() - 1);

  /// Exact device_unid
  smartstart::Query q4 = smartstart::Query(smartstart::QueryType::exact,
                                           smartstart::QueryKey::device_unid,
                                           DEVICE_15_UNID);
  std::vector<smartstart::Entry> query_result4
    = smartstart::Management::get_instance()->get_cache_entries(q4);
  TEST_ASSERT_EQUAL_INT(query_result4.size(), 1);
}

/// Test query on SmartStart list formed by false entries
void test_query_on_false_entries()
{
  std::string smartstart_list
    = generate_smartstart_list(test_entries_with_false);
  my_mqtt_on_list_update("", smartstart_list.c_str(), smartstart_list.size());
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(smartstart_enable, false);
  smartstart::Query q(smartstart::QueryType::partial,
                      smartstart::QueryKey::dsk,
                      partial_dsk_to_get_all_entries);
  std::vector<smartstart::Entry> query_result
    = smartstart::Management::get_instance()->get_cache_entries(q);
  TEST_ASSERT_EQUAL_INT(query_result.size(), test_entries_with_false.size());
  TEST_ASSERT_EQUAL(
    smartstart::Management::get_instance()->has_entries_awaiting_inclusion(),
    false);
}

/// Test whether corresponding MQTT API is called when node_added and node_removed
void test_notify_node_added_removed_sunny_cases()
{
  std::string smartstart_list
    = generate_smartstart_list(test_entries_with_mostly_true);
  my_mqtt_on_list_update("", smartstart_list.c_str(), smartstart_list.size());
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(smartstart_enable, true);
  std::string expected_message_added
    = "{\"DSK\":\"" + test_entries_with_mostly_true[0].dsk + "\",\"Unid\":\""
      + DEVICE_16_UNID + "\"}";
  uic_mqtt_publish_Expect(SMARTSTART_LIST_UPDATE_TOPIC,
                          expected_message_added.c_str(),
                          expected_message_added.size(),
                          false);
  sl_status_t status_added
    = smartstart::Management::get_instance()->notify_node_added(
      test_entries_with_mostly_true[0].dsk,
      DEVICE_16_UNID);
  TEST_ASSERT_EQUAL(status_added, SL_STATUS_OK);

  std::string expected_message_removed = "{\"DSK\":\""
                                         + test_entries_with_mostly_true[1].dsk
                                         + "\",\"Unid\":\"\"}";
  uic_mqtt_publish_Expect(SMARTSTART_LIST_UPDATE_TOPIC,
                          expected_message_removed.c_str(),
                          expected_message_removed.size(),
                          false);
  sl_status_t removed_status
    = smartstart::Management::get_instance()->notify_node_removed(
      DEVICE_15_UNID);
  TEST_ASSERT_EQUAL(removed_status, SL_STATUS_OK);
}

void test_changing_manual_intervention_required()
{
  std::string smartstart_list
    = generate_smartstart_list(test_entries_with_mostly_true);
  my_mqtt_on_list_update("", smartstart_list.c_str(), smartstart_list.size());
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(smartstart_enable, true);
  std::string expected_message = "{\"DSK\":\""
                                 + test_entries_with_mostly_true[0].dsk
                                 + "\",\"ManualInterventionRequired\":true}";

  uic_mqtt_publish_Expect(SMARTSTART_LIST_UPDATE_TOPIC,
                          expected_message.c_str(),
                          expected_message.size(),
                          false);
  sl_status_t status_update
    = smartstart::Management::get_instance()->set_manual_intervention_required(
      test_entries_with_mostly_true[0].dsk,
      true);
  TEST_ASSERT_EQUAL(status_update, SL_STATUS_OK);

  // Nothing will happen if re-update the same value as in the list
  status_update
    = smartstart::Management::get_instance()->set_manual_intervention_required(
      test_entries_with_mostly_true[0].dsk,
      false);
  TEST_ASSERT_EQUAL(status_update, SL_STATUS_OK);

  // Non-existing DSK
  status_update
    = smartstart::Management::get_instance()->set_manual_intervention_required(
      std::string("Not a DSK"),
      true);
  TEST_ASSERT_EQUAL(status_update, SL_STATUS_NOT_FOUND);
}

void test_setting_manual_intervention_required_in_json()
{
  const std::string true_str  = "true";
  const std::string false_str = "false";

  const std::string smartstart_list =
      "{\"value\": [{\"DSK\":\"" + test_entries_with_mostly_true[0].dsk + "\","
    + "\"Include\":true,"
    + "\"ProtocolControllerUnid\":\"" + test_entries_with_mostly_true[0].protocol_controller_unid + "\","
    + "\"Unid\":\"" + test_entries_with_mostly_true[0].device_unid + "\","
    + "\"ManualInterventionRequired\":" + true_str + "},"
    + "{\"DSK\":\"" + test_entries_with_mostly_true[1].dsk + "\","
    + "\"Include\":true,"
    + "\"ProtocolControllerUnid\":\"" + test_entries_with_mostly_true[1].protocol_controller_unid + "\","
    + "\"Unid\":\"" + test_entries_with_mostly_true[1].device_unid + "\","
    + "\"ManualInterventionRequired\":" + false_str + "}]}";

  my_mqtt_on_list_update("", smartstart_list.c_str(), smartstart_list.size());
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(smartstart_enable, true);

  smartstart::Query q_true
    = smartstart::Query(smartstart::QueryType::exact,
                        smartstart::QueryKey::dsk,
                        test_entries_with_mostly_true[0].dsk);
  std::vector<smartstart::Entry> query_result_true
    = smartstart::Management::get_instance()->get_cache_entries(q_true);
  TEST_ASSERT_EQUAL_INT(query_result_true[0].manual_intervention_required, true);

  smartstart::Query q_false
    = smartstart::Query(smartstart::QueryType::exact,
                        smartstart::QueryKey::dsk,
                        test_entries_with_mostly_true[1].dsk);
  std::vector<smartstart::Entry> query_result_false
    = smartstart::Management::get_instance()->get_cache_entries(q_false);
  TEST_ASSERT_EQUAL_INT(query_result_false[0].manual_intervention_required, false);
}

void test_preffered_protocol()
{
  const std::string smartstart_list =
      "{\"value\": [{\"DSK\":\"" + test_entries_with_mostly_true[0].dsk + "\","
    + "\"Include\":true,"
    + "\"ProtocolControllerUnid\":\"" + test_entries_with_mostly_true[0].protocol_controller_unid + "\","
    + "\"Unid\":\"" + test_entries_with_mostly_true[0].device_unid + "\","
    + "\"PreferredProtocols\":[\"Z-Wave\"]},"
    + "{\"DSK\":\"" + test_entries_with_mostly_true[1].dsk + "\","
    + "\"Include\":true,"
    + "\"ProtocolControllerUnid\":\"" + test_entries_with_mostly_true[1].protocol_controller_unid + "\","
    + "\"Unid\":\"" + test_entries_with_mostly_true[1].device_unid + "\","
    + "\"PreferredProtocols\":[\"Z-Wave\",\"Z-Wave Long Range\"]},"
    + "{\"DSK\":\"" + test_entries_with_mostly_true[2].dsk + "\","
    + "\"Include\":true,"
    + "\"ProtocolControllerUnid\":\"" + test_entries_with_mostly_true[2].protocol_controller_unid + "\","
    + "\"Unid\":\"" + test_entries_with_mostly_true[2].device_unid + "\","
    + "\"PreferredProtocols\":[\"Z-Wave Long Range\"]}]}";

  my_mqtt_on_list_update("", smartstart_list.c_str(), smartstart_list.size());
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(smartstart_enable, true);

  {
    smartstart::Query q_1
      = smartstart::Query(smartstart::QueryType::exact,
                          smartstart::QueryKey::dsk,
                          test_entries_with_mostly_true[0].dsk);
    std::vector<smartstart::Entry> query_result_1
      = smartstart::Management::get_instance()->get_cache_entries(q_1);
    TEST_ASSERT_EQUAL_STRING(query_result_1[0].preferred_protocols[0].c_str(), "Z-Wave");
  }

  {
    smartstart::Query q_2
      = smartstart::Query(smartstart::QueryType::exact,
                          smartstart::QueryKey::dsk,
                          test_entries_with_mostly_true[1].dsk);
    std::vector<smartstart::Entry> query_result_2
      = smartstart::Management::get_instance()->get_cache_entries(q_2);
    TEST_ASSERT_EQUAL(2, query_result_2[0].preferred_protocols.size());
    TEST_ASSERT_EQUAL_STRING(query_result_2[0].preferred_protocols[0].c_str(), "Z-Wave");
    TEST_ASSERT_EQUAL_STRING(query_result_2[0].preferred_protocols[1].c_str(), "Z-Wave Long Range");
  }

  {
    smartstart::Query q_3
      = smartstart::Query(smartstart::QueryType::exact,
                          smartstart::QueryKey::dsk,
                          test_entries_with_mostly_true[2].dsk);
    std::vector<smartstart::Entry> query_result_3
      = smartstart::Management::get_instance()->get_cache_entries(q_3);
    TEST_ASSERT_EQUAL_STRING(query_result_3[0].preferred_protocols[0].c_str(), "Z-Wave Long Range");
  }
}

/// Test whether handling invalid query correctly
void test_notify_node_added_removed_rainy_cases()
{
  std::string nonexisting_dsk
    = "76543-64107-46202-12845-60475-62452-54892-59867";
  std::string nonexisting_unid = "zw-FEEBFEEB-001";
  std::string smartstart_list
    = generate_smartstart_list(test_entries_with_mostly_true);
  my_mqtt_on_list_update("", smartstart_list.c_str(), smartstart_list.size());
  contiki_test_helper_run(0);

  sl_status_t status_added
    = smartstart::Management::get_instance()->notify_node_added(
      nonexisting_dsk,
      nonexisting_unid);
  TEST_ASSERT_EQUAL(status_added, SL_STATUS_FAIL);

  sl_status_t removed_status
    = smartstart::Management::get_instance()->notify_node_removed(
      nonexisting_unid);
  TEST_ASSERT_EQUAL(removed_status, SL_STATUS_FAIL);
}

void test_on_list_update_empty_message()
{
  std::string smartstart_list = "";
  my_mqtt_on_list_update("", smartstart_list.c_str(), smartstart_list.size());
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(smartstart_enable, false);
}

void test_on_list_update_error_message()
{
  std::string smartstart_list = "{\"DSKIncludeProtocolUnid\":\"123\"}";
  my_mqtt_on_list_update("", smartstart_list.c_str(), smartstart_list.size());
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(smartstart_enable, false);

  std::string smartstart_list2 = "[][][]";
  my_mqtt_on_list_update("", smartstart_list2.c_str(), smartstart_list2.size());
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(smartstart_enable, false);
}

void test_init_error_condition()
{
  smartstart::Management::get_instance()->teardown();
  TEST_ASSERT_EQUAL(
    smartstart::Management::get_instance()->init(MY_UNID, nullptr)
      == SL_STATUS_FAIL,
    true);
  uic_mqtt_subscribe_Ignore();
  smartstart::Management::get_instance()->init(MY_UNID,
                                               &on_entries_awaiting_inclusion);
}
}