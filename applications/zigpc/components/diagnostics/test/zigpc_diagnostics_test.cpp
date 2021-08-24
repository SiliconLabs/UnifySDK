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

#include <unistd.h>

// Diagnostic Component
#include <diagnostics_workaround.hpp>
#include <stdio.h>

extern "C" {
// Test framework
#include "unity.h"
#include "uic_mqtt_mock.h"

// UIC Component
#include "sl_status.h"
void suiteSetUp(void) {}

void setUp(void)
{
  uic_mqtt_mock_Init();
}

void tearDown(void)
{
  uic_mqtt_mock_Destroy();
}

int suiteTearDown(int numfail)
{
  return numfail;
}

//------ mqtt helper testing ------

//uic_diagnostic_request_to_strings should correct parse a json string
// and return the appropriate vector of split parsed strings
void test_uic_diagnostic_request_to_strings_correct_return()
{
  printf("test_uic_diagnostic_request_to_strings_correct_return\n");
  std::string input_json_string_payload
    = "[\"FirstMetricId\",\"SecondMetricID\",\"ThirdMetricID\"]";
  std::vector<std::string> expected_vector;
  expected_vector.push_back("FirstMetricId");
  expected_vector.push_back("SecondMetricID");
  expected_vector.push_back("ThirdMetricID");

  std::vector<std::string> output_vector
    = uic_diagnostic_request_to_strings(&input_json_string_payload);
  TEST_ASSERT_TRUE(output_vector == expected_vector);
}

// Testing diagnostic manager :

// Setup for the handle_metricID_request
// Global Flag
bool static stub_metric_flag = false;

class zigpc_stub_metric : public zigpc_diagnostics_metric
{
  private:
  public:
  zigpc_stub_metric(zigpc_diagnostics_notification &notif,
                    std::string metric_id) :
    zigpc_diagnostics_metric(notif, metric_id)
  {}

  void update_value()
  {
    stub_metric_flag = true;
    printf("Stub Metric update called\n");
  }
};

// handle_metricID_request(std::vector<std::string> metric_id_vector) should call the correct metric based on identifier string
void test_uic_diagnostic_manager_handle_metricID_request()
{
  // Create Diagnostic manager instance, vector, basic metric :
  zigpc_diagnostics_manager &manager
    = zigpc_diagnostics_manager::get_instance();
  zigpc_diagnostics_notification &m_notify = manager;
  std::vector<std::string> metric_id_v;
  printf("Starting test \n");
  std::string stub_metric_id = "SYS_HEALTH_STUB_SYNC";
  // add the metric ID to the vector
  metric_id_v.push_back(stub_metric_id);
  // Add metric to the manager map
  manager.add_metric(
    std::make_shared<zigpc_stub_metric>(m_notify, stub_metric_id));
  // Call a handlemetric request with the vector containing the good metricID
  manager.handle_metricID_request(metric_id_v);
  // check if update of the metric stub has been called
  TEST_ASSERT_TRUE(stub_metric_flag);
}

// check the increment
// test zigpc_uptime
void test_uic_diagnostics_uptime_metric()
{
  // ARRANGE
  zigpc_diagnostics_notification notif;
  zigpc_uptime_metric uptime_metric(notif, "UPTIME_MOCK_ID");
  std::string topic = uptime_metric.get_metric_id();
  long time1;
  long time2;

  // EXPECT
  uic_mqtt_publish_Expect(topic.c_str(), nullptr, 0, false);
  uic_mqtt_publish_IgnoreArg_message();
  uic_mqtt_publish_IgnoreArg_message_length();

  //ACT
  uptime_metric.update_value();
  time1 = stoi(uptime_metric.get_serialized_value());

  // 2 second delay
  usleep(2000000);

  //EXPECT
  uic_mqtt_publish_Expect(topic.c_str(), nullptr, 0, false);
  uic_mqtt_publish_IgnoreArg_message();
  uic_mqtt_publish_IgnoreArg_message_length();

  //ACT
  uptime_metric.update_value();
  time2 = stoi(uptime_metric.get_serialized_value());

  //ASSERT
  TEST_ASSERT_TRUE(time1 < time2);
}

void test_uic_diagnostics_publish_all_metric()
{
  // ARRANGE
  std::string expected_full_topic
    = "ucl/by-unid/Unleash_Kraken/ProtocolController/Diagnostics/"
      "SupportedMetrics";
  std::string stub_metric_id   = "SYS_HEALTH_STUB_SYNC";
  std::string expected_message = R"({"value":["SYS_HEALTH_STUB_SYNC"]})";
  printf(expected_full_topic.c_str());

  // prepare manager

  zigpc_diagnostics_notification notif;
  zigpc_diagnostics_manager &manager
    = zigpc_diagnostics_manager::get_instance();
  zigpc_diagnostics_notification &m_notify = manager;
  manager.set_unid("Unleash_Kraken");
  manager.add_metric(
    std::make_shared<zigpc_stub_metric>(m_notify, stub_metric_id));

  // EXPECT

  uic_mqtt_publish_Expect(expected_full_topic.c_str(),
                          expected_message.c_str(),
                          expected_message.size(),
                          true);

  //ACT

  manager.publish_metric_list();
  TEST_ASSERT_TRUE(true);
}
}
