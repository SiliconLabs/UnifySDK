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

#include "sys/sysinfo.h"

// Test framework
#include "unity.h"
#include "unity_helpers.h"
#include "uic_mqtt_mock.h"
#include "zigpc_gateway_mock.h"

// Unify Component
#include "sl_status.h"

// A global flag to check if the test/stub metric called "update_value"
bool static stub_metric_invoked_flag = false;
//Global notify flag
bool static stub_manager_invoked_flag = false;

void suiteSetUp(void) {}

void setUp(void)
{
  // set all the stub flags to false
  stub_manager_invoked_flag = false;
  stub_metric_invoked_flag  = false;
  uic_mqtt_mock_Init();
}

void tearDown(void)
{
  uic_mqtt_mock_Verify();
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
    stub_metric_invoked_flag = true;
  }
};

// stub notify

class zigpc_diagnostics_stub_manager :
  public virtual zigpc_diagnostics_notification
{
  public:
  void notify(std::string metric_id)
  {
    stub_manager_invoked_flag = true;
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
  std::string stub_metric_id = "SYS_HEALTH_STUB_SYNC";
  // add the metric ID to the vector
  metric_id_v.push_back(stub_metric_id);
  // Add metric to the manager map
  manager.add_metric(
    std::make_shared<zigpc_stub_metric>(m_notify, stub_metric_id));
  // Call a handlemetric request with the vector containing the good metricID
  manager.handle_metricID_request(metric_id_v);
  // check if update of the metric stub has been called
  TEST_ASSERT_TRUE(stub_metric_invoked_flag);
}

// check the increment
// test zigpc_uptime
void test_uic_diagnostics_uptime_metric()
{
  // ARRANGE
  zigpc_diagnostics_stub_manager notif;
  zigpc_uptime_metric uptime_metric(notif, "UPTIME_MOCK_ID");
  std::string topic = uptime_metric.get_metric_id();
  long time1;
  long time2;

  // EXPECT
  //ACT
  uptime_metric.update_value();
  time1 = stoi(uptime_metric.get_serialized_value());

  TEST_ASSERT_TRUE(stub_manager_invoked_flag);
  // Reset flag for second part of the test
  stub_manager_invoked_flag = false;

  // 2 second delay
  usleep(2000000);

  //EXPECT

  //ACT
  uptime_metric.update_value();
  time2 = stoi(uptime_metric.get_serialized_value());

  //ASSERT
  TEST_ASSERT_TRUE(time1 < time2);
  TEST_ASSERT_TRUE(stub_manager_invoked_flag);
}

void test_uic_diagnostics_publish_all_metric()
{
  // ARRANGE
  std::string expected_full_topic
    = "ucl/by-unid/Unleash_Kraken/ProtocolController/SystemHealth/"
      "SupportedMetrics";
  std::string stub_metric_id   = "SYS_HEALTH_STUB_SYNC";
  std::string expected_message = R"({"value":["SYS_HEALTH_STUB_SYNC"]})";

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

void test_uic_diagnostics_counter_metric()
{
  // ARRANGE

  zigpc_diagnostics_stub_manager notif;
  zigpc_counter_plugin_metric counter_metric(notif, "COUNTER_MOCK_ID");
  std::string topic = counter_metric.get_metric_id();

  uint16_t mock_counter_array[] = {1, 2, 3};

  //Expect
  zigpc_gateway_get_counters_capacity_ExpectAndReturn(3);

  zigpc_gateway_get_counters_list_ExpectAndReturn(NULL, 3, SL_STATUS_OK);
  zigpc_gateway_get_counters_list_IgnoreArg_list();
  zigpc_gateway_get_counters_list_ReturnArrayThruPtr_list(mock_counter_array,
                                                          3);

  zigpc_gateway_get_counters_entry_label_ExpectAndReturn(0, "FIRST");
  zigpc_gateway_get_counters_entry_label_ExpectAndReturn(1, "SECOND");
  zigpc_gateway_get_counters_entry_label_ExpectAndReturn(2, "LAST");

  //ACT
  counter_metric.update_value();

  std::string expected_message = R"({"FIRST":1,"SECOND":2,"LAST":3})";
  TEST_ASSERT_TRUE(stub_manager_invoked_flag);
  TEST_ASSERT_EQUAL_JSON(expected_message.c_str(),
                         counter_metric.get_serialized_value().c_str());
}

void test_uic_diagnostics_cpu_load_metric()
{
  // ARRANGE
  zigpc_diagnostics_stub_manager notif;
  zigpc_cpu_load_metric cpu_load_metric(notif, "CpuLoad");

  //ACT
  cpu_load_metric.update_value();

  TEST_ASSERT_TRUE(stub_manager_invoked_flag);

  TEST_ASSERT_TRUE(
    cpu_load_metric.get_value(zigpc_cpu_load_metric::load_type::AVG_1_MIN)
    > 0.0);
}

void test_uic_diagnostics_ram_usage_metric(void)
{
  zigpc_diagnostics_stub_manager notif;
  zigpc_mem_usage_metric metric(notif, "RamUsage");

  // ARRANGE

  // ACT
  metric.update_value();

  // ASSERT
  TEST_ASSERT_TRUE(metric.get_value() > 0.0);
}

void test_uic_diagnostics_neighbor_table(void){
  zigpc_diagnostics_stub_manager notif;
  zigpc_neighbor_metric metric(notif, "NeighborTable");

  // ARRANGE
  uint8_t mock_neighbor_eui64[] = {1, 2, 3,4,5,6,7,8};

  // ACT
  

  zigpc_gateway_get_neighbor_count_ExpectAndReturn(1);

  zigpc_gateway_get_neighbor_eui64_ExpectAndReturn(0, NULL, SL_STATUS_OK);
  zigpc_gateway_get_neighbor_eui64_IgnoreArg_eui64();
  zigpc_gateway_get_neighbor_eui64_ReturnArrayThruPtr_eui64(mock_neighbor_eui64,
                                                          8);

  metric.update_value();                                                        
  // ASSERT
  std::string expected_message = R"([[1,2,3,4,5,6,7,8]])";
  TEST_ASSERT_TRUE(stub_manager_invoked_flag);
  TEST_ASSERT_EQUAL_JSON(expected_message.c_str(),
                         metric.get_serialized_value().c_str());
}

}  // extern "C"
