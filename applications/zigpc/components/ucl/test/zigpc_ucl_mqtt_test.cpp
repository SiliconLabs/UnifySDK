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

#include "unity_cpp_includes.hpp"

extern "C" {

#include <unity.h>
#include <sl_status.h>
#include <uic_mqtt_mock.h>

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
void setUp(void)
{
  uic_mqtt_mock_Init();
}

/**
 * @brief Teardown after each test case
 *
 */
void tearDown(void)
{
  uic_mqtt_mock_Destroy();
}

void test_mqtt_build_unid_sanity(void)
{
  // ACT
  std::string unid_zeroes = zigpc_ucl::mqtt::build_unid(0);
  std::string unid_full   = zigpc_ucl::mqtt::build_unid(0x0D0E0A0D0B0E0E0F);

  // ASSERT
  TEST_ASSERT_EQUAL_STRING("zb-0000000000000000", unid_zeroes.c_str());
  TEST_ASSERT_EQUAL_STRING("zb-0D0E0A0D0B0E0E0F", unid_full.c_str());
}
void test_mqtt_parse_unid_sanity(void)
{
  // ACT
  zigbee_eui64_uint_t eui64;
  sl_status_t status
    = zigpc_ucl::mqtt::parse_unid("zb-0D0E0AAD0B0E0E0F", eui64);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_HEX64(0x0D0E0AAD0B0E0E0F, eui64);
}

void test_mqtt_parse_unid_should_detect_invalid_signature(void)
{
  // ACT
  zigbee_eui64_uint_t eui64;
  sl_status_t status_empty = zigpc_ucl::mqtt::parse_unid("", eui64);
  sl_status_t status_unknown_prefix
    = zigpc_ucl::mqtt::parse_unid("Test", eui64);
  sl_status_t status_no_eui64 = zigpc_ucl::mqtt::parse_unid("zb-", eui64);
  sl_status_t status_short    = zigpc_ucl::mqtt::parse_unid("zb-1234", eui64);
  sl_status_t status_no_prefix
    = zigpc_ucl::mqtt::parse_unid("0D0E0A0D0B0E0E0F", eui64);
  sl_status_t status_diff_prefix
    = zigpc_ucl::mqtt::parse_unid("azb-0D0E0A0D0B0E0E0F", eui64);
  sl_status_t status_missing_one
    = zigpc_ucl::mqtt::parse_unid("zb-010203040506070", eui64);
  sl_status_t status_invalid_hex
    = zigpc_ucl::mqtt::parse_unid("zb-010203040506070W", eui64);
  sl_status_t status_invalid_hex_long
    = zigpc_ucl::mqtt::parse_unid("zb-010203040506070WZZZ", eui64);
  sl_status_t status_valid_hex_danging
    = zigpc_ucl::mqtt::parse_unid("zb-0102030405060708HELLOW", eui64);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_SIGNATURE, status_empty);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_SIGNATURE, status_no_eui64);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_SIGNATURE, status_unknown_prefix);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_SIGNATURE, status_short);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_SIGNATURE, status_no_prefix);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_SIGNATURE, status_diff_prefix);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_SIGNATURE, status_missing_one);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_SIGNATURE, status_invalid_hex);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_SIGNATURE, status_invalid_hex_long);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_INVALID_SIGNATURE, status_valid_hex_danging);
}

void test_parse_payload_should_reject_invalid_json(void)
{
  bpt::ptree pt;

  // ACT
  sl_status_t status_null    = zigpc_ucl::mqtt::parse_payload(nullptr, pt);
  sl_status_t status_empty   = zigpc_ucl::mqtt::parse_payload("", pt);
  sl_status_t status_invalid = zigpc_ucl::mqtt::parse_payload("{", pt);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status_null);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_EMPTY, status_empty);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OBJECT_READ, status_invalid);
}

void test_parse_payload_should_accept_empty_json(void)
{
  bpt::ptree pt;

  // ACT
  sl_status_t status = zigpc_ucl::mqtt::parse_payload("{}", pt);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  TEST_ASSERT_TRUE(pt.empty());
}

void test_parse_payload_should_accept_valid_json(void)
{
  bpt::ptree pt;

  // ACT
  sl_status_t status = zigpc_ucl::mqtt::parse_payload("{\"a\": 2}", pt);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  TEST_ASSERT_FALSE(pt.empty());
  TEST_ASSERT_EQUAL(2, pt.get<int>("a"));
}

void test_build_topic_sanity(void)
{
  std::string topic;
  zigpc_ucl::mqtt::topic_data_t data = {
    .eui64 = 0x1234567812345678,
  };

  // ACT
  sl_status_t status = zigpc_ucl::mqtt::build_topic(
    zigpc_ucl::mqtt::topic_type_t::BY_UNID_PC_NWMGMT,
    data,
    topic);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_STRING(
    "ucl/by-unid/zb-1234567812345678/ProtocolController/NetworkManagement",
    topic.c_str());
}

void test_subscribe_reject_invalid_args(void)
{
  zigpc_ucl::mqtt::topic_data_t data = {
    .eui64 = 0x1234567812345678,
  };

  // ACT
  sl_status_t status = zigpc_ucl::mqtt::subscribe(
    zigpc_ucl::mqtt::topic_type_t::BY_UNID_PC_NWMGMT,
    data,
    nullptr);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
}

void mqtt_cb_stub(const char *topic,
                  const char *message,
                  const size_t message_length)
{}

void test_subscribe_sanity(void)
{
  // ARRANGE
  zigpc_ucl::mqtt::topic_data_t data = {
    .eui64 = 0x1234567812345678,
  };

  uic_mqtt_subscribe_Expect("ucl/by-unid/zb-1234567812345678/"
                            "ProtocolController/NetworkManagement/Write",
                            mqtt_cb_stub);

  // ACT
  sl_status_t status = zigpc_ucl::mqtt::subscribe(
    zigpc_ucl::mqtt::topic_type_t::BY_UNID_PC_NWMGMT_WRITE,
    data,
    mqtt_cb_stub);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_publish_sanity(void)
{
  // ARRANGE
  zigpc_ucl::mqtt::topic_data_t data = {
    .eui64 = 0x1234567812345678,
  };

  std::string payload = "";

  uic_mqtt_publish_Expect(
    "ucl/by-unid/zb-1234567812345678/ProtocolController/NetworkManagement",
    payload.c_str(),
    payload.size(),
    true);

  // ACT
  sl_status_t status
    = zigpc_ucl::mqtt::publish(zigpc_ucl::mqtt::topic_type_t::BY_UNID_PC_NWMGMT,
                               data,
                               payload.c_str(),
                               payload.size(),
                               true);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

}  // extern "C"
