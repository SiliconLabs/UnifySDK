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
#include "zigpc_attrmgmt_int.hpp"

extern "C" {

#include "unity.h"

#include "zcl_attribute_info_mock.h"
#include "zcl_definitions.h"
#include "zigpc_common_zigbee_mock.h"
#include "zigpc_gateway_mock.h"

#include "attribute_management_int.h"

static const size_t TEST_SEND_LIMIT
  = ZIGPC_ATTRMGMT_REPORT_RECORDS_LIMIT_PER_MESSAGE;

/**
 * @brief Setup the test suite (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp(void) {}

/**
 * @brief Teardown the test suite (called once after all test_xxx functions are called)
 *
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/**
 * @brief Setup before each test case
 *
 */
void setUp(void) {}

/**
 * @brief Teardown after each test case
 *
 */
void tearDown(void) {}

void test_records_to_send_in_message_sanity(void)
{
  // ARRANGE
  size_t total_count = 10;
  size_t limit       = 3;
  size_t start_index = 0;
  // ACT
  size_t to_send = zigpc_attrmgmt_get_records_to_send_in_message(start_index,
                                                                 total_count,
                                                                 limit);
  // ASSERT
  TEST_ASSERT_EQUAL(limit, to_send);
}

void test_records_to_send_in_message_last_set(void)
{
  // ARRANGE
  size_t total_count = 11;
  size_t limit       = 7;
  size_t start_index = 8;
  // ACT
  size_t to_send = zigpc_attrmgmt_get_records_to_send_in_message(start_index,
                                                                 total_count,
                                                                 limit);
  // ASSERT
  TEST_ASSERT_EQUAL(3, to_send);
}

void test_records_to_send_in_message_exceed_count(void)
{
  // ARRANGE
  size_t total_count = 11;
  size_t limit       = 7;
  size_t start_index = 13;
  // ACT
  size_t to_send = zigpc_attrmgmt_get_records_to_send_in_message(start_index,
                                                                 total_count,
                                                                 limit);
  // ASSERT
  TEST_ASSERT_EQUAL(0, to_send);
}

void test_report_build_should_return_on_unsupported_cluster(void)
{
  // ARRANGE
  sl_status_t status;
  zcl_cluster_id_t cluster_id = 0x1234;
  std::vector<zigpc_zcl_configure_reporting_record_t> records;

  zigpc_zclprofiles_get_cluster_attribute_count_ExpectAndReturn(cluster_id, 0);

  // ACT
  status = zigpc_attrmgmt_build_configure_report_records(cluster_id, records);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_HEX(0, records.size());
}

void test_report_build_should_include_all_reportable_attributes(void)
{
  // ARRANGE
  sl_status_t status;
  zcl_cluster_id_t cluster_id = 0x1234;
  size_t attr_count           = 12;
  std::vector<zcl_attribute_t> attr_info_list;
  std::vector<zigpc_zcl_configure_reporting_record_t> records;

  for (size_t i = 0; i < attr_count; i++) {
    attr_info_list.push_back({
      cluster_id,
      static_cast<zcl_attribute_id_t>(i),
      (enum e_zcl_attribute_type)ZIGPC_ZCL_DATA_TYPE_UINT16,
      true,
    });
  }

  zigpc_zclprofiles_get_cluster_attribute_count_ExpectAndReturn(
    cluster_id,
    attr_info_list.size());

  zigpc_zclprofiles_get_cluster_attribute_list_ExpectAndReturn(cluster_id,
                                                               NULL,
                                                               SL_STATUS_OK);
  zigpc_zclprofiles_get_cluster_attribute_list_IgnoreArg_attribute_list();
  zigpc_zclprofiles_get_cluster_attribute_list_ReturnArrayThruPtr_attribute_list(
    attr_info_list.data(),
    attr_info_list.size());

  // ACT
  status = zigpc_attrmgmt_build_configure_report_records(cluster_id, records);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(attr_count, records.size());
  for (size_t i = 0; i < records.size(); i++) {
    TEST_ASSERT_EQUAL(ZIGPC_ZCL_GLOBAL_REPORTING_ROLE_GENERATOR,
                      records[i].direction);
    TEST_ASSERT_EQUAL(attr_info_list[i].attribute_id,
                      records[i].attribute_identifier);
    TEST_ASSERT_EQUAL(attr_info_list[i].type, records[i].attribute_type);
  }
}

void test_report_build_should_not_include_non_reportable_attributes(void)
{
  // ARRANGE
  sl_status_t status;
  zcl_cluster_id_t cluster_id = 0x1234;
  size_t attr_count           = 4;
  std::vector<zcl_attribute_t> attr_info_list;
  std::vector<zigpc_zcl_configure_reporting_record_t> records;

  for (size_t i = 0; i < attr_count; i++) {
    attr_info_list.push_back({
      cluster_id,
      static_cast<zcl_attribute_id_t>(i),
      (enum e_zcl_attribute_type)ZIGPC_ZCL_DATA_TYPE_UINT16,
      (i % 2) ? false : true,  // every other attribute is not reportable
    });
  }

  zigpc_zclprofiles_get_cluster_attribute_count_ExpectAndReturn(
    cluster_id,
    attr_info_list.size());

  zigpc_zclprofiles_get_cluster_attribute_list_ExpectAndReturn(cluster_id,
                                                               NULL,
                                                               SL_STATUS_OK);
  zigpc_zclprofiles_get_cluster_attribute_list_IgnoreArg_attribute_list();
  zigpc_zclprofiles_get_cluster_attribute_list_ReturnArrayThruPtr_attribute_list(
    attr_info_list.data(),
    attr_info_list.size());

  // ACT
  status = zigpc_attrmgmt_build_configure_report_records(cluster_id, records);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(attr_count / 2, records.size());

  for (size_t i = 0; i < attr_count; i += 2) {
    TEST_ASSERT_EQUAL_HEX16(attr_info_list[i].attribute_id,
                            records[i / 2].attribute_identifier);
  }
}

void test_report_split_should_sanitize_inputs(void)
{
  // ARRANGE
  sl_status_t status;
  std::vector<zigpc_zcl_configure_reporting_record_t> records;

  // ACT
  status = zigpc_attrmgmt_send_split_report_config_cmds(NULL, 1, 0, records);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
}

void test_report_split_should_send_one_command_for_one_record(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_eui64_t eui64 = {0x04, 0x84, 0x02, 0xF2, 0x0A, 0x4C, 0x99, 0x10};
  zigbee_endpoint_id_t endpoint_id = 2;
  zcl_cluster_id_t cluster_id      = 0x2334;
  std::vector<zigpc_zcl_configure_reporting_record_t> records;

  records.push_back({
    ZIGPC_ZCL_GLOBAL_REPORTING_ROLE_GENERATOR,  // direction
    0xFFFF,                                     // attribute_identifier
    ZIGPC_ZCL_DATA_TYPE_UINT8,                  // attribute_type
    10,                                         // minimum_reporting_interval
    20,                                         // maximum_reporting_interval
    1,                                          // reportable_change
  });

  zigpc_gateway_configure_reports_ExpectAndReturn(
    eui64,
    endpoint_id,
    cluster_id,
    reinterpret_cast<const uint8_t *>(records.data()),
    sizeof(zigpc_zcl_configure_reporting_record_t),
    SL_STATUS_OK);

  // ACT
  status = zigpc_attrmgmt_send_split_report_config_cmds(eui64,
                                                        endpoint_id,
                                                        cluster_id,
                                                        records);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_report_split_should_send_one_command_for_records_with_less_than_send_limit_count(
  void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_eui64_t eui64 = {0x04, 0x54, 0x02, 0xF2, 0x0A, 0x4C, 0x99, 0x10};
  zigbee_endpoint_id_t endpoint_id = 2;
  zcl_cluster_id_t cluster_id      = 0x2334;
  size_t record_count              = TEST_SEND_LIMIT - 1;
  std::vector<zigpc_zcl_configure_reporting_record_t> records;

  for (uint8_t i = 0; i < record_count; i++) {
    records.push_back({
      ZIGPC_ZCL_GLOBAL_REPORTING_ROLE_GENERATOR,    // direction
      static_cast<zcl_attribute_id_t>(0xFFFF - i),  // attribute_identifier
      ZIGPC_ZCL_DATA_TYPE_UINT8,                    // attribute_type
      static_cast<uint16_t>(i * 2),  // minimum_reporting_interval
      static_cast<uint16_t>(i * 4),  // maximum_reporting_interval
      i,                             // reportable_change
    });
  }

  zigpc_gateway_configure_reports_ExpectAndReturn(
    eui64,
    endpoint_id,
    cluster_id,
    reinterpret_cast<const uint8_t *>(records.data()),
    sizeof(zigpc_zcl_configure_reporting_record_t) * record_count,
    SL_STATUS_OK);

  // ACT
  status = zigpc_attrmgmt_send_split_report_config_cmds(eui64,
                                                        endpoint_id,
                                                        cluster_id,
                                                        records);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_report_split_should_send_one_command_for_records_with_send_limit_count(
  void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_eui64_t eui64 = {0x04, 0x54, 0x02, 0xF2, 0x0A, 0x4C, 0x99, 0x10};
  zigbee_endpoint_id_t endpoint_id = 2;
  zcl_cluster_id_t cluster_id      = 0x2334;
  size_t record_count              = TEST_SEND_LIMIT;
  std::vector<zigpc_zcl_configure_reporting_record_t> records;

  for (uint8_t i = 0; i < record_count; i++) {
    records.push_back({
      ZIGPC_ZCL_GLOBAL_REPORTING_ROLE_GENERATOR,    // direction
      static_cast<zcl_attribute_id_t>(0xFFFF - i),  // attribute_identifier
      ZIGPC_ZCL_DATA_TYPE_UINT8,                    // attribute_type
      static_cast<uint16_t>(i * 2),  // minimum_reporting_interval
      static_cast<uint16_t>(i * 4),  // maximum_reporting_interval
      i,                             // reportable_change
    });
  }

  zigpc_gateway_configure_reports_ExpectAndReturn(
    eui64,
    endpoint_id,
    cluster_id,
    reinterpret_cast<const uint8_t *>(records.data()),
    sizeof(zigpc_zcl_configure_reporting_record_t) * record_count,
    SL_STATUS_OK);

  // ACT
  status = zigpc_attrmgmt_send_split_report_config_cmds(eui64,
                                                        endpoint_id,
                                                        cluster_id,
                                                        records);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_report_split_should_send_two_commands_for_records_with_greater_than_send_limit_count(
  void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_eui64_t eui64 = {0x04, 0x54, 0x02, 0xF2, 0x0A, 0x4C, 0x99, 0x10};
  zigbee_endpoint_id_t endpoint_id = 2;
  zcl_cluster_id_t cluster_id      = 0x2334;
  size_t record_count              = TEST_SEND_LIMIT + 1;
  std::vector<zigpc_zcl_configure_reporting_record_t> records;

  for (uint8_t i = 0; i < record_count; i++) {
    records.push_back({
      ZIGPC_ZCL_GLOBAL_REPORTING_ROLE_GENERATOR,    // direction
      static_cast<zcl_attribute_id_t>(0xFFFF - i),  // attribute_identifier
      ZIGPC_ZCL_DATA_TYPE_UINT8,                    // attribute_type
      static_cast<uint16_t>(i * 2),  // minimum_reporting_interval
      static_cast<uint16_t>(i * 4),  // maximum_reporting_interval
      i,                             // reportable_change
    });
  }

  zigpc_gateway_configure_reports_ExpectAndReturn(
    eui64,
    endpoint_id,
    cluster_id,
    reinterpret_cast<const uint8_t *>(records.data()),
    sizeof(zigpc_zcl_configure_reporting_record_t) * TEST_SEND_LIMIT,
    SL_STATUS_OK);

  // Expect sending second set of records
  zigpc_gateway_configure_reports_ExpectAndReturn(
    eui64,
    endpoint_id,
    cluster_id,
    reinterpret_cast<const uint8_t *>(records.data() + TEST_SEND_LIMIT),
    sizeof(zigpc_zcl_configure_reporting_record_t) * 1,
    SL_STATUS_OK);

  // ACT
  status = zigpc_attrmgmt_send_split_report_config_cmds(eui64,
                                                        endpoint_id,
                                                        cluster_id,
                                                        records);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_report_split_should_send_many_commands_for_records_with_greater_than_send_limit_count(
  void)
{
  // ARRANGE
  sl_status_t status;
  size_t expected_msg_count = 11;
  zigbee_eui64_t eui64      = {0x04, 0x54, 0x02, 0xF2, 0x0A, 0x4C, 0x99, 0x10};
  zigbee_endpoint_id_t endpoint_id = 2;
  zcl_cluster_id_t cluster_id      = 0x2334;
  size_t record_count              = TEST_SEND_LIMIT * expected_msg_count + 1;
  std::vector<zigpc_zcl_configure_reporting_record_t> records;

  for (uint8_t i = 0; i < record_count; i++) {
    records.push_back({
      ZIGPC_ZCL_GLOBAL_REPORTING_ROLE_GENERATOR,    // direction
      static_cast<zcl_attribute_id_t>(0xFFFF - i),  // attribute_identifier
      ZIGPC_ZCL_DATA_TYPE_UINT8,                    // attribute_type
      static_cast<uint16_t>(i * 2),  // minimum_reporting_interval
      static_cast<uint16_t>(i * 4),  // maximum_reporting_interval
      i,                             // reportable_change
    });
  }

  for (size_t i = 0U; i < expected_msg_count; i++) {
    zigpc_gateway_configure_reports_ExpectAndReturn(
      eui64,
      endpoint_id,
      cluster_id,
      reinterpret_cast<const uint8_t *>(records.data() + (i * TEST_SEND_LIMIT)),
      sizeof(zigpc_zcl_configure_reporting_record_t) * TEST_SEND_LIMIT,
      SL_STATUS_OK);
  }

  // Expect sending last record
  zigpc_gateway_configure_reports_ExpectAndReturn(
    eui64,
    endpoint_id,
    cluster_id,
    reinterpret_cast<const uint8_t *>(records.data()
                                      + (TEST_SEND_LIMIT * expected_msg_count)),
    sizeof(zigpc_zcl_configure_reporting_record_t) * 1,
    SL_STATUS_OK);

  // ACT
  status = zigpc_attrmgmt_send_split_report_config_cmds(eui64,
                                                        endpoint_id,
                                                        cluster_id,
                                                        records);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_endpoint_configure_reports_should_handle_invalid_inputs(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_eui64_t eui64       = {0xA4, 0x54, 0x02, 0xF2, 0x0A, 0x4C, 0x99, 0x10};
  zigbee_endpoint_t endpoint = {0};

  is_valid_zigbee_endpoint_ExpectAndReturn(endpoint, true);

  // ACT
  status = zigpc_attrmgmt_configure_reports_for_endpoint(NULL, endpoint);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);

  // ARRANGE
  is_valid_zigbee_endpoint_ExpectAndReturn(endpoint, false);

  // ACT
  status = zigpc_attrmgmt_configure_reports_for_endpoint(eui64, endpoint);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_PARAMETER, status);
}

void test_endpoint_configure_reports_one_cluster_sanity(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_eui64_t eui64      = {0xA4, 0x54, 0x02, 0xF2, 0x0A, 0x4C, 0x99, 0x10};
  unsigned int record_count = 5;
  std::vector<zcl_attribute_t> attr_info_list;
  zigbee_endpoint_t endpoint = {
    .endpoint_id   = 2,
    .cluster_list  = {{.cluster_id = 0x2133}},
    .cluster_count = 1,
  };
  zcl_cluster_id_t cluster_id = endpoint.cluster_list[0].cluster_id;

  for (uint8_t i = 0; i < record_count; i++) {
    attr_info_list.push_back({
      0x2133,                                                // cluster_id
      static_cast<zcl_attribute_id_t>(0xFFFF - i),           // attribute_id
      (enum e_zcl_attribute_type)ZIGPC_ZCL_DATA_TYPE_UINT8,  // type
      true,                                                  // is_reportable
    });
  }

  is_valid_zigbee_endpoint_ExpectAndReturn(endpoint, true);

  zigpc_zclprofiles_get_cluster_attribute_count_ExpectAndReturn(cluster_id,
                                                                record_count);

  zigpc_zclprofiles_get_cluster_attribute_list_ExpectAndReturn(cluster_id,
                                                               NULL,
                                                               SL_STATUS_OK);
  zigpc_zclprofiles_get_cluster_attribute_list_IgnoreArg_attribute_list();
  zigpc_zclprofiles_get_cluster_attribute_list_ReturnArrayThruPtr_attribute_list(
    attr_info_list.data(),
    attr_info_list.size());

  // Expect functions called in zigpc_attrmgmt_send_split_report_config_cmds()
  zigpc_gateway_configure_reports_ExpectAndReturn(
    eui64,
    endpoint.endpoint_id,
    cluster_id,
    nullptr,
    sizeof(zigpc_zcl_configure_reporting_record_t) * record_count,
    SL_STATUS_OK);
  zigpc_gateway_configure_reports_IgnoreArg_report_record();

  // ACT
  status = zigpc_attrmgmt_configure_reports_for_endpoint(eui64, endpoint);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

}  // extern "C"
