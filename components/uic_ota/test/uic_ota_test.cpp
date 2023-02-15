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

// Standard library
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

// Test framework
#include "unity.h"
#include "unity_helpers.h"

// This component
#include "workaround.hpp"

// Unify imports
#include "contiki_test_helper.h"
#include "sl_status.h"
#include "sl_log.h"
#include "mqtt_test_helper.h"

std::string construct_status_update_topic(const dotdot_unid_t &unid,
                                          const dotdot_endpoint_id_t &endpoint,
                                          const ota_uiid_t &uiid,
                                          const std::string status_type)
{
  return "ucl/by-unid/" + std::string(unid) + "/ep" + std::to_string(endpoint)
         + "/OTA/Attributes/UIID/" + std::string(uiid) + "/" + status_type;
}

extern "C" {
#include "uic_mqtt_mock.h"
#define LOG_TAG "OTA_TEST"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

uic_ota::meta_t meta_info;
uint8_t info_callback_counter = 0;
void image_available_function_callback(const uic_ota::meta_t &meta)
{
  meta_info.version = meta.version;
  meta_info.uiid    = meta.uiid;
  meta_info.unid    = meta.unid;
  TEST_ASSERT_FALSE(meta_info.uiid.empty());
  meta_info.apply_after = meta.apply_after;
  meta_info.filename    = meta.filename;
  info_callback_counter++;
}

static unsigned long int cache_size = 256 * 10 ^ 6;
static uint32_t image_timeout       = 4;
static std::string image_dir        = "/tmp";

// Called before each and every test
void setUp()
{
  contiki_test_helper_init();
  contiki_test_helper_run(0);
  mqtt_test_helper_init();
  uic_ota::init(image_available_function_callback,
                image_dir,
                cache_size,
                image_timeout);
}

void tearDown()
{
  uic_ota::clear_cache();
  contiki_test_helper_run(image_timeout * CLOCK_SECOND);
}

void test_ota_subscribes_and_callback()
{
  // Subscribe to uiid and all unids
  const char topic[]       = "ucl/OTA/info/uiid/all";
  std::string payload_meta = R"({
    "Version": "0.0.1",
    "ApplyAfter": "2021-07-13T12:13:59+05:00",
    "Filename": "ZW_PowerStrip_7.16.0_90_ZGM130S_REGION_US_LR.gbl"
  })";

  // 1
  // Check that meta info is updated when a MQTT message is received on info
  // topic for a UIID
  uic_ota::subscribe_unid("unid", "uiid");
  mqtt_test_helper_publish(topic, payload_meta.c_str(), payload_meta.length());

  TEST_ASSERT_EQUAL_STRING("0.0.1", meta_info.version.c_str());
  TEST_ASSERT_EQUAL_STRING("uiid", meta_info.uiid.c_str());
  TEST_ASSERT_EQUAL_STRING("unid", meta_info.unid.c_str());
  TEST_ASSERT_EQUAL_STRING("ZW_PowerStrip_7.16.0_90_ZGM130S_REGION_US_LR.gbl",
                           meta_info.filename.c_str());

  // 2
  // Check that unsusbcription works by unsusbscribing and checking meta info
  // is not updated
  meta_info.unid        = "";
  meta_info.uiid        = "";
  meta_info.apply_after = 0;
  meta_info.version     = "0.0.0";

  uic_ota::unsubscribe_unid("unid", "uiid");
  mqtt_test_helper_publish(topic, payload_meta.c_str(), payload_meta.length());

  TEST_ASSERT_EQUAL_STRING("0.0.0", meta_info.version.c_str());
  TEST_ASSERT_EQUAL_STRING("", meta_info.uiid.c_str());
  TEST_ASSERT_EQUAL_STRING("", meta_info.unid.c_str());
  TEST_ASSERT_EQUAL(0, meta_info.apply_after);

  // 3
  // Check that subscription works for a UNID and UIID by subscribing and checking
  // that meta info is not updated
  const char topic_2[] = "ucl/OTA/info/uiid/unid";
  uic_ota::subscribe_unid("unid", "uiid");

  payload_meta = R"({
    "Version": "0.1.0",
    "ApplyAfter": "2021-07-13T15:13:59+02:00",
    "Filename": "ZW_PowerStrip_7.16.0_90_ZGM130S_REGION_EU_LR.gbl"
  })";

  mqtt_test_helper_publish(topic_2,
                           payload_meta.c_str(),
                           payload_meta.length());
  TEST_ASSERT_EQUAL_STRING("0.1.0", meta_info.version.c_str());
  TEST_ASSERT_EQUAL_STRING("uiid", meta_info.uiid.c_str());
  TEST_ASSERT_EQUAL_STRING("unid", meta_info.unid.c_str());
  TEST_ASSERT_EQUAL_STRING("ZW_PowerStrip_7.16.0_90_ZGM130S_REGION_EU_LR.gbl",
                           meta_info.filename.c_str());

  // 4
  // Unsusbscribe from UIID and UNID and check meta info is not updated
  meta_info.unid        = "";
  meta_info.uiid        = "";
  meta_info.apply_after = 0;
  meta_info.version     = "0.0.0";

  uic_ota::unsubscribe_unid("unid", "uiid");

  mqtt_test_helper_publish(topic_2,
                           payload_meta.c_str(),
                           payload_meta.length());

  TEST_ASSERT_EQUAL_STRING("0.0.0", meta_info.version.c_str());
  TEST_ASSERT_EQUAL_STRING("", meta_info.uiid.c_str());
  TEST_ASSERT_EQUAL_STRING("", meta_info.unid.c_str());
  TEST_ASSERT_EQUAL(0, meta_info.apply_after);

  // 5
  // Wrong date-time string
  const char topic_3[] = "ucl/OTA/info/uiid_3/unid_3";
  uic_ota::subscribe_unid("unid_3", "uiid_3");

  payload_meta = R"({
    "Version": "0.1.0",
    "ApplyAfter": "date-time",
    "Filename": "ZW_PowerStrip_7.16.0_90_ZGM130S_REGION_EU_LR.gbl"
  })";

  mqtt_test_helper_publish(topic_3,
                           payload_meta.c_str(),
                           payload_meta.length());

  // Should be the same as before
  TEST_ASSERT_EQUAL_STRING("0.0.0", meta_info.version.c_str());
  TEST_ASSERT_EQUAL_STRING("", meta_info.uiid.c_str());
  TEST_ASSERT_EQUAL_STRING("", meta_info.unid.c_str());
  TEST_ASSERT_EQUAL(0, meta_info.apply_after);

  // 6
  // Wrong payload but valid json
  uic_ota::subscribe_unid("unid_4", "uiid_4");
  const char topic_4[] = "ucl/OTA/info/uiid_4/unid_4";
  payload_meta         = R"({
    "Filename": "ZW_PowerStrip_7.16.0_90_ZGM130S_REGION_EU_LR.gbl"
  })";

  mqtt_test_helper_publish(topic_4,
                           payload_meta.c_str(),
                           payload_meta.length());

  // Should be the same as before
  TEST_ASSERT_EQUAL_STRING("0.0.0", meta_info.version.c_str());
  TEST_ASSERT_EQUAL_STRING("", meta_info.uiid.c_str());
  TEST_ASSERT_EQUAL_STRING("", meta_info.unid.c_str());
  TEST_ASSERT_EQUAL(0, meta_info.apply_after);

  // 7
  // Invalid JSON
  uic_ota::subscribe_unid("unid_5", "uiid_5");
  const char topic_5[] = "ucl/OTA/info/uiid_5/unid_5";
  payload_meta         = R"(
    "Version": "0.1.0"
  })";

  mqtt_test_helper_publish(topic_5,
                           payload_meta.c_str(),
                           payload_meta.length());

  // Should be the same as before
  TEST_ASSERT_EQUAL_STRING("0.0.0", meta_info.version.c_str());
  TEST_ASSERT_EQUAL_STRING("", meta_info.uiid.c_str());
  TEST_ASSERT_EQUAL_STRING("", meta_info.unid.c_str());
  TEST_ASSERT_EQUAL(0, meta_info.apply_after);

  // 8
  // Incorrect apply after
  uic_ota::subscribe_unid("unid_6", "uiid_6");
  const char topic_6[] = "ucl/OTA/info/uiid_6/unid_6";
  payload_meta         = R"({
    "Version": "0.1.0",
    "ApplyAfter": "2021-07-13T15:13:59+",
    "Filename": "ZW_PowerStrip_7.16.0_90_ZGM130S_REGION_EU_LR.gbl"
  })";

  mqtt_test_helper_publish(topic_6,
                           payload_meta.c_str(),
                           payload_meta.length());

  // Should be the same as before
  TEST_ASSERT_EQUAL_STRING("0.0.0", meta_info.version.c_str());
  TEST_ASSERT_EQUAL_STRING("", meta_info.uiid.c_str());
  TEST_ASSERT_EQUAL_STRING("", meta_info.unid.c_str());
  TEST_ASSERT_EQUAL(0, meta_info.apply_after);

  // 9
  // Only version in payload
  uic_ota::subscribe_unid("unid_9", "uiid_9");
  const char topic_9[] = "ucl/OTA/info/uiid_9/unid_9";
  payload_meta         = R"({
    "Version": "0.1.0"
  })";

  mqtt_test_helper_publish(topic_9,
                           payload_meta.c_str(),
                           payload_meta.length());

  TEST_ASSERT_EQUAL_STRING("0.1.0", meta_info.version.c_str());
  TEST_ASSERT_EQUAL_STRING("uiid_9", meta_info.uiid.c_str());
  TEST_ASSERT_EQUAL_STRING("unid_9", meta_info.unid.c_str());
  TEST_ASSERT_EQUAL_STRING("Firmware_image_unid_9_uiid_9",
                           meta_info.filename.c_str());
  TEST_ASSERT_EQUAL(0, meta_info.apply_after);

  // 10
  // Version and filename
  uic_ota::subscribe_unid("unid_10", "uiid_10");
  const char topic_10[] = "ucl/OTA/info/uiid_10/unid_10";
  payload_meta          = R"({
    "Version": "0.1.0",
    "Filename": "ZW_PowerStrip_7.16.0_90_ZGM130S_REGION_EU_LR.gbl"
  })";

  mqtt_test_helper_publish(topic_10,
                           payload_meta.c_str(),
                           payload_meta.length());

  // Should be the same as before
  TEST_ASSERT_EQUAL_STRING("0.1.0", meta_info.version.c_str());
  TEST_ASSERT_EQUAL_STRING("uiid_10", meta_info.uiid.c_str());
  TEST_ASSERT_EQUAL_STRING("unid_10", meta_info.unid.c_str());
  TEST_ASSERT_EQUAL(0, meta_info.apply_after);

  // 11
  // Version and filename
  uic_ota::subscribe_unid("unid_11", "uiid_11");
  const char topic_11[] = "ucl/OTA/info/uiid_11/unid_11";
  payload_meta          = R"({
    "Version": "0.1.0",
    "ApplyAfter": "2021-07-13T15:13:59+06:00"
  })";

  mqtt_test_helper_publish(topic_11,
                           payload_meta.c_str(),
                           payload_meta.length());

  // Should be the same as before
  TEST_ASSERT_EQUAL_STRING("0.1.0", meta_info.version.c_str());
  TEST_ASSERT_EQUAL_STRING("uiid_11", meta_info.uiid.c_str());
  TEST_ASSERT_EQUAL_STRING("unid_11", meta_info.unid.c_str());
  TEST_ASSERT_NOT_EQUAL(0, meta_info.apply_after);

  // 12
  // Multiple unids listening to UIID all check counter info callback
  const char topic_12[]       = "ucl/OTA/info/uiid_12/all";
  std::string payload_meta_12 = R"({
    "Version": "0.0.1",
    "ApplyAfter": "2021-07-13T12:13:59+05:00",
    "Filename": "ZW_PowerStrip_7.16.0_90_ZGM130S_REGION_US_LR.gbl"
  })";
  info_callback_counter       = 0;
  uic_ota::subscribe_unid("unid_12_1", "uiid_12");
  uic_ota::subscribe_unid("unid_12_2", "uiid_12");
  uic_ota::subscribe_unid("unid_12_3", "uiid_12");
  uic_ota::subscribe_unid("unid_12_4", "uiid_12");
  mqtt_test_helper_publish(topic_12,
                           payload_meta_12.c_str(),
                           payload_meta_12.length());

  TEST_ASSERT_EQUAL_STRING("0.0.1", meta_info.version.c_str());
  TEST_ASSERT_EQUAL_STRING("uiid_12", meta_info.uiid.c_str());
  TEST_ASSERT_EQUAL_STRING("unid_12_4", meta_info.unid.c_str());
  TEST_ASSERT_EQUAL_STRING("ZW_PowerStrip_7.16.0_90_ZGM130S_REGION_US_LR.gbl",
                           meta_info.filename.c_str());
  TEST_ASSERT_EQUAL(4, info_callback_counter);

  // 13
  // Unsubscribe all uiids which a unid is subscribed to.
  const char topic_13[]       = "ucl/OTA/info/uiid_13/all";
  std::string payload_meta_13 = R"({
    "Version": "1.0.0",
    "ApplyAfter": "2021-07-13T12:13:59+02:00",
    "Filename": "ZW_test.gbl"
  })";
  meta_info.unid              = "";
  meta_info.uiid              = "";
  meta_info.apply_after       = 0;
  meta_info.version           = "0.0.0";
  info_callback_counter       = 0;
  uic_ota::subscribe_unid("unid_13", "uiid_13_1");
  uic_ota::subscribe_unid("unid_13", "uiid_13_2");
  uic_ota::subscribe_unid("unid_13", "uiid_13_3");
  uic_ota::subscribe_unid("unid_13", "uiid_13_4");

  uic_ota::unsubscribe_all_unids_uiid("unid_13");
  mqtt_test_helper_publish(topic_13,
                           payload_meta_13.c_str(),
                           payload_meta_13.length());

  TEST_ASSERT_EQUAL_STRING("0.0.0", meta_info.version.c_str());
  TEST_ASSERT_EQUAL_STRING("", meta_info.uiid.c_str());
  TEST_ASSERT_EQUAL_STRING("", meta_info.unid.c_str());
  TEST_ASSERT_EQUAL(0, info_callback_counter);
}

void time_zone_string(std::string &offset_string, long offset)
{
  offset_string = "+";
  if (offset < 0) {
    offset_string = "-";
    offset        = -offset;
  }

  char number[10];
  uint16_t minutes = offset / 60;
  uint8_t hours    = int(minutes / 60);
  minutes          = int(minutes % 60);

  std::snprintf(number, sizeof(number), "%02d", hours);
  offset_string += std::string(number);
  std::snprintf(number, sizeof(number), "%02d", minutes);
  offset_string += ":" + std::string(number);
}

time_t get_time_from_string(std::string time, std::string format)
{
  struct tm tm = {0};
  tm.tm_isdst  = -1;
  strptime(time.c_str(), format.c_str(), &tm);
  return mktime(&tm);
}

void test_ota_timer_apply_after()
{
  std::string format = "%Y-%m-%dT%T%z";

  std::string test_offset_string;
  time_zone_string(test_offset_string, 1 * 3600);

  // Subscribe to uiid and all unids
  const char topic[]       = "ucl/OTA/info/uiid/all";
  std::string payload_meta = R"({
    "Version": "0.0.1",
    "ApplyAfter": "$021-00-00T00:00:00+00:00",
    "Filename": "ZW_PowerStrip_7.16.0_90_ZGM130S_REGION_US_LR.gbl"
  })";

  // 1
  // Ofsetting positive one hour from current time zone
  std::string time_1 = "2021-07-13T12:13:59" + test_offset_string;
  payload_meta.replace(payload_meta.find("$"), time_1.length(), time_1.c_str());

  uic_ota::subscribe_unid("unid", "uiid");
  mqtt_test_helper_publish(topic, payload_meta.c_str(), payload_meta.length());

  std::string date     = "2021-07-13T11:13:59+00:00";
  time_t expected_time = get_time_from_string(date, format);

  TEST_ASSERT_EQUAL_MESSAGE(expected_time,
                            meta_info.apply_after,
                            "1 hour + offset failed");

  // 2
  // Offsetting one hour from current time zone in negative direction
  payload_meta = R"({
    "Version": "0.0.1",
    "ApplyAfter": "$021-00-00T00:00:00+00:00",
    "Filename": "ZW_PowerStrip_7.16.0_90_ZGM130S_REGION_US_LR.gbl"
  })";

  time_zone_string(test_offset_string, -1 * 3600 - 120);

  time_1 = "2021-07-13T12:13:59" + test_offset_string;

  payload_meta.replace(payload_meta.find("$"), time_1.length(), time_1);

  mqtt_test_helper_publish(topic, payload_meta.c_str(), payload_meta.length());

  std::string date_2     = "2021-07-13T13:15:59+00:00";
  time_t expected_time_2 = get_time_from_string(date_2, format);

  TEST_ASSERT_EQUAL_MESSAGE(expected_time_2,
                            meta_info.apply_after,
                            "1 hour - offset failed");

  // 3
  // Offsetting 12 hours from current time zone in negative direction
  std::string payload_meta_3 = R"({
    "Version": "0.0.1",
    "ApplyAfter": "$021-00-00T00:00:00+00:00",
    "Filename": "ZW_PowerStrip_7.16.0_90_ZGM130S_REGION_US_LR.gbl"
  })";

  std::string test_offset_string_3;
  time_zone_string(test_offset_string_3, -12 * 3600 - 120);

  std::string time_3 = "2021-07-13T12:13:59" + test_offset_string_3;

  payload_meta.replace(payload_meta_3.find("$"), time_3.length(), time_3);

  mqtt_test_helper_publish(topic, payload_meta.c_str(), payload_meta.length());

  std::string date_3 = "2021-07-14T00:15:59+00:00";
  struct tm tm_3     = {0};
  tm_3.tm_isdst      = -1;
  strptime(date_3.c_str(), format.c_str(), &tm_3);
  time_t expected_time_3 = mktime(&tm_3);

  TEST_ASSERT_EQUAL_MESSAGE(expected_time_3,
                            meta_info.apply_after,
                            "OFfsetting - 12 hours and - 2 minutes");

  // 4
  // Offsetting 12 hours from current time zone in positive direction
  std::string payload_meta_4 = R"({
    "Version": "0.0.1",
    "ApplyAfter": "$021-00-00T00:00:00+00:00",
    "Filename": "ZW_PowerStrip_7.16.0_90_ZGM130S_REGION_US_LR.gbl"
  })";

  std::string test_offset_string_4;
  time_zone_string(test_offset_string_4, 13 * 3600 + 120);

  std::string time_4 = "2021-07-13T12:13:59" + test_offset_string_4;
  payload_meta.replace(payload_meta_4.find("$"), time_4.length(), time_4);

  mqtt_test_helper_publish(topic, payload_meta.c_str(), payload_meta.length());

  std::string date_4     = "2021-07-12T23:11:59+00:00";
  time_t expected_time_4 = get_time_from_string(date_4, format);

  TEST_ASSERT_EQUAL_MESSAGE(expected_time_4,
                            meta_info.apply_after,
                            "Offsetting + 12 hours and + 2 minutes");

  // 5
  // Same time zone
  std::string payload_meta_5 = R"({
    "Version": "0.0.1",
    "ApplyAfter": "$021-00-00T00:00:00+00:00",
    "Filename": "ZW_PowerStrip_7.16.0_90_ZGM130S_REGION_US_LR.gbl"
  })";

  std::string time_5 = "2021-07-13T12:13:59+00:00";
  payload_meta.replace(payload_meta_5.find("$"), time_5.length(), time_5);

  mqtt_test_helper_publish(topic, payload_meta.c_str(), payload_meta.length());

  std::string date_5     = "2021-07-13T12:13:59+00:00";
  time_t expected_time_5 = get_time_from_string(date_5, format);

  TEST_ASSERT_EQUAL_MESSAGE(expected_time_5,
                            meta_info.apply_after,
                            "No offset in time from UTC failed");
}

uint32_t image_ready_callback_1_counter         = 0;
uint32_t image_ready_callback_2_counter         = 0;
uint32_t image_ready_callback_1_timeout_counter = 0;
uint32_t image_ready_callback_2_timeout_counter = 0;
uint32_t image_ready_callback_1_error_counter   = 0;
uint32_t image_ready_callback_2_error_counter   = 0;
std::string file_path_result                    = "";
uic_ota::image_ready_result_t image_result
  = uic_ota::image_ready_result_t::ERROR;
void image_ready_callback_1(uic_ota::image_ready_result_t result,
                            std::string file_path)
{
  sl_log_debug(LOG_TAG, "Image ready callback counter 1");
  image_result = result;
  if (image_result == uic_ota::image_ready_result_t::TIMED_OUT) {
    image_ready_callback_1_timeout_counter++;
    sl_log_debug(LOG_TAG, "Ready 1 timeout");
  } else if (image_result == uic_ota::image_ready_result_t::ERROR) {
    image_ready_callback_1_error_counter++;
  }
  image_ready_callback_1_counter++;
  file_path_result = file_path;
}

void image_ready_callback_2(uic_ota::image_ready_result_t result,
                            std::string file_path)
{
  sl_log_debug(LOG_TAG, "Image ready callback counter 2");
  image_result = result;
  if (image_result == uic_ota::image_ready_result_t::TIMED_OUT) {
    image_ready_callback_2_timeout_counter++;
    sl_log_debug(LOG_TAG, "Ready 2 timeout");
  } else if (image_result == uic_ota::image_ready_result_t::ERROR) {
    image_ready_callback_2_error_counter++;
  }
  image_ready_callback_2_counter++;

  file_path_result = file_path;
}

void reset_counters()
{
  image_ready_callback_1_counter         = 0;
  image_ready_callback_2_counter         = 0;
  image_ready_callback_1_timeout_counter = 0;
  image_ready_callback_2_timeout_counter = 0;
}

void image_dummy_meta_info(const std::string image_key)
{
  std::string topic = (image_key.find("/") != std::string::npos)
                        ? "ucl/OTA/info/" + image_key
                        : "ucl/OTA/info/" + image_key + "/all";

  sl_log_debug(LOG_TAG, "Publishing dummy meta info %s", topic.c_str());

  std::string payload_meta = R"({
    "Version": "0.0.1",
    "ApplyAfter": "2021-07-13T12:13:59+05:00",
    "Filename": "dummy.gbl"
  })";

  mqtt_test_helper_publish(topic.c_str(),
                           payload_meta.c_str(),
                           payload_meta.length());
}

void test_ota_image_ready_uiid()
{
  // Subscribe to uiid and all unids
  const char image_provider_topic_1[] = "ucl/OTA/data/UIC-000-000-001_EU/all";

  // 1
  // Check image ready is called after receiving payload
  reset_counters();
  uic_ota::subscribe_unid("unid", "UIC-000-000-001_EU");
  image_dummy_meta_info("UIC-000-000-001_EU");
  uic_ota::get_by_unid("unid", "UIC-000-000-001_EU", image_ready_callback_1);

  std::string image_payload = "Image";
  mqtt_test_helper_publish(image_provider_topic_1,
                           image_payload.c_str(),
                           image_payload.length());

  TEST_ASSERT_EQUAL_MESSAGE(uic_ota::image_ready_result_t::OK,
                            image_result,
                            "Expected success on callback UIC-000-000-001_EU");
  TEST_ASSERT_EQUAL(1, image_ready_callback_1_counter);
  contiki_test_helper_run(image_timeout * CLOCK_SECOND);

  // 2
  // Testing multiple uiids callbacks
  reset_counters();
  const char image_provider_topic_2[] = "ucl/OTA/data/UIC-000-000-002_EU/all";
  image_result                        = uic_ota::image_ready_result_t::ERROR;
  uic_ota::subscribe_unid("unid", "UIC-000-000-002_EU");
  image_dummy_meta_info("UIC-000-000-002_EU");
  uic_ota::get_by_unid("unid", "UIC-000-000-002_EU", image_ready_callback_2);

  image_payload = "Image";
  mqtt_test_helper_publish(image_provider_topic_2,
                           image_payload.c_str(),
                           image_payload.length());

  TEST_ASSERT_EQUAL(uic_ota::image_ready_result_t::OK, image_result);
  TEST_ASSERT_EQUAL(0, image_ready_callback_1_counter);
  TEST_ASSERT_EQUAL(1, image_ready_callback_2_counter);
  contiki_test_helper_run(image_timeout * CLOCK_SECOND);

  // 3
  // Callback is overwritten
  reset_counters();
  uic_ota::subscribe_unid("unid", "UIC-000-000-012_EU");
  image_dummy_meta_info("UIC-000-000-012_EU");
  uic_ota::get_by_unid("unid", "UIC-000-000-012_EU", image_ready_callback_1);
  uic_ota::get_by_unid("unid", "UIC-000-000-012_EU", image_ready_callback_2);
  const char image_provider_topic_3[] = "ucl/OTA/data/UIC-000-000-012_EU/all";

  mqtt_test_helper_publish(image_provider_topic_3,
                           image_payload.c_str(),
                           image_payload.length());

  contiki_test_helper_run(image_timeout * CLOCK_SECOND);

  TEST_ASSERT_EQUAL(0, image_ready_callback_1_counter);
  TEST_ASSERT_EQUAL(1, image_ready_callback_2_counter);
  TEST_ASSERT_EQUAL(0, image_ready_callback_1_timeout_counter);
  TEST_ASSERT_EQUAL(0, image_ready_callback_2_timeout_counter);

  // 4
  // Testing having multiple unids listening to UIID
  reset_counters();
  uic_ota::subscribe_unid("unid", "UIC-000-000-012_EU");
  uic_ota::subscribe_unid("unid_2", "UIC-000-000-012_EU");
  uic_ota::subscribe_unid("unid_3", "UIC-000-000-012_EU");
  uic_ota::subscribe_unid("unid_4", "UIC-000-000-012_EU");
  image_dummy_meta_info("UIC-000-000-012_EU");
  uic_ota::get_by_unid("unid", "UIC-000-000-012_EU", image_ready_callback_1);
  uic_ota::get_by_unid("unid_2", "UIC-000-000-012_EU", image_ready_callback_2);
  uic_ota::get_by_unid("unid_3", "UIC-000-000-012_EU", image_ready_callback_1);
  uic_ota::get_by_unid("unid_4", "UIC-000-000-012_EU", image_ready_callback_2);
  uic_ota::get_by_unid("unid_5", "UIC-000-000-012_EU", image_ready_callback_2);

  mqtt_test_helper_publish(image_provider_topic_3,
                           image_payload.c_str(),
                           image_payload.length());

  contiki_test_helper_run(image_timeout * CLOCK_SECOND);

  TEST_ASSERT_EQUAL(2, image_ready_callback_1_counter);
  TEST_ASSERT_EQUAL(3, image_ready_callback_2_counter);
  TEST_ASSERT_EQUAL(0, image_ready_callback_1_timeout_counter);
  TEST_ASSERT_EQUAL(1, image_ready_callback_2_timeout_counter);
  TEST_ASSERT_EQUAL(uic_ota::image_ready_result_t::TIMED_OUT, image_result);
  contiki_test_helper_run(image_timeout * CLOCK_SECOND);
}

void test_ota_image_ready_unid()
{
  reset_counters();

  // Subscribe to specific unids
  const char image_provider_topic_1[]
    = "ucl/OTA/data/UIC-000-000-003_EU/UIC-C87E6FB9-0003";

  // 1
  // Image ready unid simple
  uic_ota::subscribe_unid("UIC-C87E6FB9-0003", "UIC-000-000-003_EU");
  image_dummy_meta_info("UIC-000-000-003_EU/UIC-C87E6FB9-0003");
  uic_ota::get_by_unid("UIC-C87E6FB9-0003",
                       "UIC-000-000-003_EU",
                       image_ready_callback_1);

  std::string image_payload = "Image";
  mqtt_test_helper_publish(image_provider_topic_1,
                           image_payload.c_str(),
                           image_payload.length());
  TEST_ASSERT_EQUAL_MESSAGE(1,
                            image_ready_callback_1_counter,
                            "First callback on unid was not called");
  TEST_ASSERT_EQUAL_MESSAGE(
    0,
    image_ready_callback_2_counter,
    "Second callback was called first should have been");
  TEST_ASSERT_EQUAL(0, image_ready_callback_1_timeout_counter);
  TEST_ASSERT_EQUAL(0, image_ready_callback_2_timeout_counter);
  contiki_test_helper_run(image_timeout * CLOCK_SECOND);

  // 2
  // Checking that multiple callbacks on multiple different unids can be
  // handled
  const char image_provider_topic_2[]
    = "ucl/OTA/data/UIC-000-000-004_EU/UIC-C87E6FB9-0004";
  reset_counters();

  uic_ota::subscribe_unid("UIC-C87E6FB9-0004", "UIC-000-000-004_EU");
  image_dummy_meta_info("UIC-000-000-004_EU/UIC-C87E6FB9-0003");
  image_dummy_meta_info("UIC-000-000-004_EU/UIC-C87E6FB9-0004");

  uic_ota::get_by_unid("UIC-C87E6FB9-0003",
                       "UIC-000-000-004_EU",
                       image_ready_callback_1);
  uic_ota::get_by_unid("UIC-C87E6FB9-0004",
                       "UIC-000-000-004_EU",
                       image_ready_callback_2);

  mqtt_test_helper_publish(image_provider_topic_2,
                           image_payload.c_str(),
                           image_payload.length());

  TEST_ASSERT_EQUAL(1, image_ready_callback_1_counter);
  TEST_ASSERT_EQUAL(1, image_ready_callback_2_counter);
  TEST_ASSERT_EQUAL(0, image_ready_callback_1_timeout_counter);
  TEST_ASSERT_EQUAL(0, image_ready_callback_2_timeout_counter);
  contiki_test_helper_run(image_timeout * CLOCK_SECOND);

  // 3
  // Checking that a callback can be overwritten by a protocol controller
  reset_counters();
  uic_ota::get_by_unid("UIC-C87E6FB9-0004",
                       "UIC-000-000-004_EU",
                       image_ready_callback_1);

  mqtt_test_helper_publish(image_provider_topic_2,
                           image_payload.c_str(),
                           image_payload.length());

  TEST_ASSERT_EQUAL_MESSAGE(
    1,
    image_ready_callback_1_counter,
    "Overwriting second callback with first did not work");
  TEST_ASSERT_EQUAL_MESSAGE(0,
                            image_ready_callback_2_counter,
                            "Overwriting second callback did not work");
  TEST_ASSERT_EQUAL(0, image_ready_callback_1_timeout_counter);
  TEST_ASSERT_EQUAL(0, image_ready_callback_2_timeout_counter);
  contiki_test_helper_run(image_timeout * CLOCK_SECOND);

  // 4
  // Make sure that images always takes the specific unid over all
  reset_counters();
  file_path_result                    = "";
  const char image_provider_topic_3[] = "ucl/OTA/data/UIC-000-000-010_EU/all";
  uic_ota::subscribe_unid("UIC-C87E6FB9-0010", "UIC-000-000-010_EU");
  image_dummy_meta_info("UIC-000-000-010_EU");
  std::string payload_meta = R"({
    "Version": "0.0.1",
    "ApplyAfter": "2021-07-13T12:13:59+05:00",
    "Filename": "dummy_3.gbl"
  })";

  mqtt_test_helper_publish("ucl/OTA/info/UIC-000-000-010_EU/UIC-C87E6FB9-0010",
                           payload_meta.c_str(),
                           payload_meta.length());

  uic_ota::get_by_unid("UIC-C87E6FB9-0010",
                       "UIC-000-000-010_EU",
                       image_ready_callback_1);

  mqtt_test_helper_publish(image_provider_topic_3,
                           image_payload.c_str(),
                           image_payload.length());

  TEST_ASSERT_EQUAL_STRING("", file_path_result.c_str());

  mqtt_test_helper_publish("ucl/OTA/data/UIC-000-000-010_EU/UIC-C87E6FB9-0010",
                           image_payload.c_str(),
                           image_payload.length());

  TEST_ASSERT_EQUAL(1, image_ready_callback_1_counter);
  TEST_ASSERT_EQUAL(0, image_ready_callback_2_counter);
  TEST_ASSERT_EQUAL(0, image_ready_callback_1_timeout_counter);
  TEST_ASSERT_EQUAL(0, image_ready_callback_2_timeout_counter);
  TEST_ASSERT_EQUAL_STRING("/tmp/UIC-000-000-010_EU_UIC-C87E6FB9-0010.dat",
                           file_path_result.c_str());
  contiki_test_helper_run(image_timeout * CLOCK_SECOND);

  // 5
  // Check that meta info is removed on unretained image info
  mqtt_test_helper_publish("ucl/OTA/info/UIC-000-000-010_EU/UIC-C87E6FB9-0010",
                           "",
                           0);
  uic_ota::get_by_unid("UIC-C87E6FB9-0010",
                       "UIC-000-000-010_EU",
                       image_ready_callback_1);
  mqtt_test_helper_publish(image_provider_topic_3,
                           image_payload.c_str(),
                           image_payload.length());
  contiki_test_helper_run(image_timeout * CLOCK_SECOND);

  TEST_ASSERT_EQUAL_STRING("/tmp/UIC-000-000-010_EU.dat",
                           file_path_result.c_str());
}

void test_ota_image_ready_status()
{
  // 1
  // Check that the timeout works if an MQTT message is not revceieved within
  // timeout on a simple UIID
  reset_counters();
  uic_ota::subscribe_unid("unid", "UIC-000-000-007_EU");
  image_dummy_meta_info("UIC-000-000-007_EU");
  uic_ota::get_by_unid("unid", "UIC-000-000-007_EU", image_ready_callback_1);
  contiki_test_helper_run(image_timeout * CLOCK_SECOND);

  TEST_ASSERT_EQUAL_MESSAGE(uic_ota::image_ready_result_t::TIMED_OUT,
                            image_result,
                            "The timeout of get image was not in effect");

  // 2
  // Check that the timeout works if an MQTT message is not received within
  // timeout on a UIID and UNID
  uic_ota::subscribe_unid("UIC-C87E6FB9-0005", "UIC-000-000-005_EU");
  image_dummy_meta_info("UIC-000-000-005_EU/UIC-C87E6FB9-0005");
  image_result = uic_ota::image_ready_result_t::OK;
  uic_ota::get_by_unid("UIC-C87E6FB9-0005",
                       "UIC-000-000-005_EU",
                       image_ready_callback_1);

  contiki_test_helper_run(image_timeout * CLOCK_SECOND);
  TEST_ASSERT_EQUAL_MESSAGE(uic_ota::image_ready_result_t::TIMED_OUT,
                            image_result,
                            "The timeout for unid get image was not in effect");

  // 3
  // Check that images stay in poll que even though image received and first
  // pops image key poll que after timeout on ctimer
  reset_counters();
  const char image_provider_topic_2[]
    = "ucl/OTA/data/UIC-000-000-005_EU/UIC-C87E6FB9-0005";
  image_dummy_meta_info("UIC-000-000-005_EU/UIC-C87E6FB9-0005");
  // uic_ota::get_by_uiid("UIC-000-000-005_EU", image_ready_callback_1);
  uic_ota::get_by_unid("UIC-C87E6FB9-0005",
                       "UIC-000-000-005_EU",
                       image_ready_callback_2);

  std::string image_payload = "Image";
  mqtt_test_helper_publish(image_provider_topic_2,
                           image_payload.c_str(),
                           image_payload.length());

  contiki_test_helper_run(image_timeout * CLOCK_SECOND);
  // TEST_ASSERT_EQUAL_MESSAGE(1,
  // image_ready_callback_1_timeout_counter,
  // "The order of image keys in poll is not correct");
  TEST_ASSERT_EQUAL(1, image_ready_callback_2_counter);

  // 4
  // Having multiple ctimers going at the same time
  reset_counters();

  uint8_t callback_timers_count = 10;

  for (uint32_t i = 0; i < callback_timers_count; ++i) {
    uic_ota::get_by_unid("UIC-C87E6FB9-0015",
                         "UIC-000-000-015_EU",
                         image_ready_callback_2);
  }

  contiki_test_helper_run(image_timeout * CLOCK_SECOND);

  TEST_ASSERT_EQUAL(callback_timers_count, image_ready_callback_2_counter);
  TEST_ASSERT_EQUAL(callback_timers_count,
                    image_ready_callback_2_error_counter);
}

void test_ota_validating_images()
{
  // 1
  // Testing if validation is succesfull on small payload and high cache size
  uic_ota::init(image_available_function_callback,
                image_dir,
                256 * 10 ^ 6,
                image_timeout);

  const char image_provider_topic_1[]
    = "ucl/OTA/data/UIC-000-000-006_EU/ZWave-000-000-006_EU";

  std::string image_payload = "Image payload";

  uic_ota::subscribe_unid("ZWave-000-000-006_EU", "UIC-000-000-006_EU");
  image_dummy_meta_info("UIC-000-000-006_EU/ZWave-000-000-006_EU");
  uic_ota::get_by_unid("ZWave-000-000-006_EU",
                       "UIC-000-000-006_EU",
                       image_ready_callback_1);
  mqtt_test_helper_publish(image_provider_topic_1,
                           image_payload.c_str(),
                           image_payload.length());

  TEST_ASSERT_EQUAL(uic_ota::image_ready_result_t::OK, image_result);
  contiki_test_helper_run(image_timeout * CLOCK_SECOND);

  // 2
  // Testing if valdiation fails if cache size is zero
  uic_ota::init(image_available_function_callback, image_dir, 0, image_timeout);
  uic_ota::get_by_unid("ZWave-000-000-016_EU",
                       "UIC-000-000-016_EU",
                       image_ready_callback_1);
  const char image_provider_topic_2[]
    = "ucl/OTA/data/UIC-000-000-016_EU/ZWave-000-000-016_EU";
  mqtt_test_helper_publish(image_provider_topic_2,
                           image_payload.c_str(),
                           image_payload.length());

  contiki_test_helper_run(image_timeout * CLOCK_SECOND);
  TEST_ASSERT_EQUAL(uic_ota::image_ready_result_t::ERROR, image_result);

  // 3
  // Argument beyond max cache limit
  uic_ota::init(image_available_function_callback,
                image_dir,
                2 * 256 * 1024 * 1024,
                image_timeout);
  uic_ota::get_by_unid("ZWave-000-000-006_EU",
                       "UIC-000-000-006_EU",
                       image_ready_callback_1);

  mqtt_test_helper_publish(image_provider_topic_1,
                           image_payload.c_str(),
                           image_payload.length());

  contiki_test_helper_run(image_timeout * CLOCK_SECOND);
  TEST_ASSERT_EQUAL(uic_ota::image_ready_result_t::OK, image_result);
}

void test_ota_status_updates()
{
  dotdot_unid_t unid            = "UIC-C87E6FB9-0006";
  dotdot_endpoint_id_t endpoint = 1;
  ota_uiid_t uiid               = "UIC-000-000-006_EU";

  char message_reported[500];
  char message_desired[500];

  // 1
  // Status update current version
  std::string status_topic_desired
    = construct_status_update_topic(unid, endpoint, uiid, "CurrentVersion")
      + "/Desired";
  std::string status_topic_reported
    = construct_status_update_topic(unid, endpoint, uiid, "CurrentVersion")
      + "/Reported";

  uic_ota::update_current_version(unid, endpoint, uiid, "0x070F01");
  mqtt_test_helper_pop_publish(status_topic_desired.c_str(), message_desired);
  mqtt_test_helper_pop_publish(status_topic_reported.c_str(), message_reported);

  TEST_ASSERT_EQUAL_JSON(R"({"value":"0x070F01"})", message_desired);
  TEST_ASSERT_EQUAL_JSON(R"({"value":"0x070F01"})", message_reported);

  // 2
  // Status update target version
  status_topic_desired
    = construct_status_update_topic(unid, endpoint, uiid, "TargetVersion")
      + "/Desired";
  status_topic_reported
    = construct_status_update_topic(unid, endpoint, uiid, "TargetVersion")
      + "/Reported";

  uic_ota::update_target_version(unid, endpoint, uiid, "0x070F01");
  mqtt_test_helper_pop_publish(status_topic_desired.c_str(), message_desired);
  mqtt_test_helper_pop_publish(status_topic_reported.c_str(), message_reported);

  TEST_ASSERT_EQUAL_JSON(R"({"value":"0x070F01"})", message_desired);
  TEST_ASSERT_EQUAL_JSON(R"({"value":"0x070F01"})", message_reported);

  // 3
  // status update apply after time
  // Getting time zone of device running unit tests
  status_topic_desired
    = construct_status_update_topic(unid, endpoint, uiid, "ApplyAfter")
      + "/Desired";
  status_topic_reported
    = construct_status_update_topic(unid, endpoint, uiid, "ApplyAfter")
      + "/Reported";

  std::string date_string = "2021-07-13T13:13:59+00:00";
  time_t time_argument    = get_time_from_string(date_string, "%Y-%m-%dT%T%z");

  uic_ota::update_apply_after(unid, endpoint, uiid, time_argument);
  mqtt_test_helper_pop_publish(status_topic_desired.c_str(), message_desired);
  mqtt_test_helper_pop_publish(status_topic_reported.c_str(), message_reported);

  std::string expected_payload = R"({"value":"2021-07-13T13:13:59+00:00"})";

  TEST_ASSERT_EQUAL_JSON(expected_payload.c_str(), message_desired);
  TEST_ASSERT_EQUAL_JSON(expected_payload.c_str(), message_reported);

  // 4
  // Status update for OTA status
  status_topic_desired
    = construct_status_update_topic(unid, endpoint, uiid, "Status")
      + "/Desired";
  status_topic_reported
    = construct_status_update_topic(unid, endpoint, uiid, "Status")
      + "/Reported";

  uic_ota::update_status(unid,
                         endpoint,
                         uiid,
                         uic_ota::status_t::DOWNLOAD_IN_PROGRESS);
  mqtt_test_helper_pop_publish(status_topic_desired.c_str(), message_desired);
  mqtt_test_helper_pop_publish(status_topic_reported.c_str(), message_reported);

  TEST_ASSERT_EQUAL_JSON(R"({"value":"DownloadInProgress"})", message_desired);
  TEST_ASSERT_EQUAL_JSON(R"({"value":"DownloadInProgress"})", message_reported);

  // 5
  // Status update for Size
  status_topic_desired
    = construct_status_update_topic(unid, endpoint, uiid, "Size") + "/Desired";
  status_topic_reported
    = construct_status_update_topic(unid, endpoint, uiid, "Size") + "/Reported";

  uic_ota::update_size(unid, endpoint, uiid, 256);
  mqtt_test_helper_pop_publish(status_topic_desired.c_str(), message_desired);
  mqtt_test_helper_pop_publish(status_topic_reported.c_str(), message_reported);

  TEST_ASSERT_EQUAL_JSON(R"({"value":256})", message_desired);
  TEST_ASSERT_EQUAL_JSON(R"({"value":256})", message_reported);

  // 6
  // Status update offset
  status_topic_desired
    = construct_status_update_topic(unid, endpoint, uiid, "Offset")
      + "/Desired";
  status_topic_reported
    = construct_status_update_topic(unid, endpoint, uiid, "Offset")
      + "/Reported";

  uic_ota::update_offset(unid, endpoint, uiid, 50);
  mqtt_test_helper_pop_publish(status_topic_desired.c_str(), message_desired);
  mqtt_test_helper_pop_publish(status_topic_reported.c_str(), message_reported);

  TEST_ASSERT_EQUAL_JSON(R"({"value":50})", message_desired);
  TEST_ASSERT_EQUAL_JSON(R"({"value":50})", message_reported);

  // 7
  // Status update Last error
  status_topic_desired
    = construct_status_update_topic(unid, endpoint, uiid, "LastError")
      + "/Desired";
  status_topic_reported
    = construct_status_update_topic(unid, endpoint, uiid, "LastError")
      + "/Reported";

  uic_ota::update_last_error(unid,
                             endpoint,
                             uiid,
                             uic_ota::last_error_t::SUCCESS);
  mqtt_test_helper_pop_publish(status_topic_desired.c_str(), message_desired);
  mqtt_test_helper_pop_publish(status_topic_reported.c_str(), message_reported);

  TEST_ASSERT_EQUAL_JSON(R"({"value":"Success"})", message_desired);
  TEST_ASSERT_EQUAL_JSON(R"({"value":"Success"})", message_reported);
}

void test_ota_supported_commands()
{
  dotdot_unid_t unid            = "UIC-C87E6FB9-0006";
  dotdot_endpoint_id_t endpoint = 1;
  ota_uiid_t uiid               = "UIC-000-000-006_EU";

  char message_reported[500];
  char message_desired[500];

  // 1
  // Status update current version and supported commands
  std::string status_topic_desired
    = construct_status_update_topic(unid, endpoint, uiid, "CurrentVersion")
      + "/Desired";
  std::string status_topic_reported
    = construct_status_update_topic(unid, endpoint, uiid, "CurrentVersion")
      + "/Reported";

  uic_ota::update_current_version(unid, endpoint, uiid, "0x070F01");
  mqtt_test_helper_pop_publish(status_topic_desired.c_str(), message_desired);
  mqtt_test_helper_pop_publish(status_topic_reported.c_str(), message_reported);

  TEST_ASSERT_EQUAL_JSON(R"({"value":"0x070F01"})", message_desired);
  TEST_ASSERT_EQUAL_JSON(R"({"value":"0x070F01"})", message_reported);

  std::string supported_commands_topic = "ucl/by-unid/" + std::string(unid)
                                         + "/ep" + std::to_string(endpoint)
                                         + "/OTA/SupportedCommands";

  sl_log_debug(LOG_TAG,
               "SupportedCommands topic: %s",
               supported_commands_topic.c_str());
  mqtt_test_helper_pop_publish(supported_commands_topic.c_str(),
                               message_desired);

  TEST_ASSERT_EQUAL_JSON(R"({"value":[]})", message_desired);
}

void test_ota_unretain_status_topics()
{
  // Clear out in advance
  uic_mqtt_unretain_Ignore();
  uic_ota::unretain_ota_status();
  sl_log_debug(LOG_TAG, "Cleared");

  dotdot_endpoint_id_t endpoint   = 1;
  dotdot_endpoint_id_t endpoint_2 = 2;

  dotdot_unid_t unid   = "UIC-C87E6FB9-01";
  ota_uiid_t uiid      = "UIC-000-000-01_EU";
  dotdot_unid_t unid_2 = "UIC-C87E6FB9-02";
  ota_uiid_t uiid_2    = "UIC-000-000-02_EU";

  // 1
  // Fill up some statuses and make sure they are unretained on clean up
  std::string status_topic_desired_current_version
    = construct_status_update_topic(unid, endpoint, uiid, "CurrentVersion")
      + "/Desired";
  std::string status_topic_desired_current_version_2
    = construct_status_update_topic(unid, endpoint_2, uiid, "CurrentVersion")
      + "/Desired";
  std::string status_topic_desired_current_version_3
    = construct_status_update_topic(unid_2, endpoint, uiid_2, "CurrentVersion")
      + "/Desired";
  std::string status_topic_desired_current_version_4
    = construct_status_update_topic(unid_2,
                                    endpoint_2,
                                    uiid_2,
                                    "CurrentVersion")
      + "/Desired";

  uic_ota::update_current_version(unid, endpoint, uiid, "0x070F01");
  uic_ota::update_current_version(unid, endpoint_2, uiid, "0x070F01");
  uic_ota::update_current_version(unid_2, endpoint, uiid_2, "0x070F01");
  uic_ota::update_current_version(unid_2, endpoint_2, uiid_2, "0x070F01");
  std::string expect_unretain_1 = "ucl/by-unid/" + std::string(unid) + "/ep"
                                  + std::to_string(endpoint) + "/OTA";
  std::string expect_unretain_2 = "ucl/by-unid/" + std::string(unid) + "/ep"
                                  + std::to_string(endpoint_2) + "/OTA";
  std::string expect_unretain_3 = "ucl/by-unid/" + std::string(unid_2) + "/ep"
                                  + std::to_string(endpoint) + "/OTA";
  std::string expect_unretain_4 = "ucl/by-unid/" + std::string(unid_2) + "/ep"
                                  + std::to_string(endpoint_2) + "/OTA";

  uic_mqtt_unretain_Expect(expect_unretain_1.c_str());
  uic_mqtt_unretain_Expect(expect_unretain_2.c_str());
  uic_mqtt_unretain_Expect(expect_unretain_3.c_str());
  uic_mqtt_unretain_Expect(expect_unretain_4.c_str());
  uic_ota::unretain_ota_status();

  // 2
  // Fill up multiple unids status and endpoints and make sure they are cleaned up again
  uic_ota::update_current_version(unid, endpoint, uiid, "0x070F01");
  uic_ota::update_current_version(unid, endpoint_2, uiid, "0x070F01");
  uic_ota::update_current_version(unid_2, endpoint, uiid_2, "0x070F01");
  uic_ota::update_current_version(unid_2, endpoint_2, uiid_2, "0x070F01");

  uic_mqtt_unretain_Expect(expect_unretain_3.c_str());
  uic_mqtt_unretain_Expect(expect_unretain_4.c_str());
  uic_ota::unretain_ota_status_by_unid(unid_2);
}

void test_ota_cached_images()
{
  const char image_provider_topic_1[]
    = "ucl/OTA/data/UIC-000-000-010_EU/UIC-C87E6FB9-0010";

  // 1
  // First normal approach get image and receive it over MQTT
  reset_counters();
  uic_ota::subscribe_unid("UIC-C87E6FB9-0010", "UIC-000-000-010_EU");
  image_dummy_meta_info("UIC-000-000-010_EU/UIC-C87E6FB9-0010");
  uic_ota::get_by_unid("UIC-C87E6FB9-0010",
                       "UIC-000-000-010_EU",
                       image_ready_callback_1);

  std::string image_payload = "Image";
  mqtt_test_helper_publish(image_provider_topic_1,
                           image_payload.c_str(),
                           image_payload.length());
  contiki_test_helper_run(image_timeout * CLOCK_SECOND);
  TEST_ASSERT_EQUAL(1, image_ready_callback_1_counter);
  TEST_ASSERT_EQUAL(0, image_ready_callback_2_counter);
  TEST_ASSERT_EQUAL(0, image_ready_callback_1_timeout_counter);
  TEST_ASSERT_EQUAL(0, image_ready_callback_2_timeout_counter);

  // 2
  // Expecting image to be cached and saved so all i have to do is call get
  uic_ota::get_by_unid("UIC-C87E6FB9-0010",
                       "UIC-000-000-010_EU",
                       image_ready_callback_1);
  uic_ota::get_by_unid("UIC-C87E6FB9-0010",
                       "UIC-000-000-010_EU",
                       image_ready_callback_1);

  contiki_test_helper_run(image_timeout * CLOCK_SECOND);
  TEST_ASSERT_EQUAL(2, image_ready_callback_1_counter);
  TEST_ASSERT_EQUAL(0, image_ready_callback_2_counter);
  TEST_ASSERT_EQUAL(1, image_ready_callback_1_timeout_counter);
  TEST_ASSERT_EQUAL(0, image_ready_callback_2_timeout_counter);

  // 3
  // Checking if file is written even though there is a file in cache
  reset_counters();
  std::string image_key = "UIC-000-000-010_EU/UIC-C87E6FB9-0010";
  std::string topic     = (image_key.find("/") != std::string::npos)
                            ? "ucl/OTA/info/" + image_key
                            : "ucl/OTA/info/" + image_key + "/all";

  std::string payload_meta = R"({
    "Version": "0.0.1",
    "ApplyAfter": "2021-07-13T12:13:59+05:00",
    "Filename": "dummy_2.gbl"
  })";

  mqtt_test_helper_publish(topic.c_str(),
                           payload_meta.c_str(),
                           payload_meta.length());

  uic_ota::get_by_unid("UIC-C87E6FB9-0010",
                       "UIC-000-000-010_EU",
                       image_ready_callback_1);
  mqtt_test_helper_publish(image_provider_topic_1,
                           image_payload.c_str(),
                           image_payload.length());
  contiki_test_helper_run(image_timeout * CLOCK_SECOND);

  bool file_exists = false;
  if (access("/tmp/UIC-000-000-010_EU_UIC-C87E6FB9-0010.dat", F_OK) == 0) {
    file_exists = true;
  }

  TEST_ASSERT_TRUE_MESSAGE(file_exists, "OTA failed to generate the file");
  TEST_ASSERT_EQUAL(1, image_ready_callback_1_counter);
  TEST_ASSERT_EQUAL(0, image_ready_callback_2_counter);
  TEST_ASSERT_EQUAL(0, image_ready_callback_1_timeout_counter);
  TEST_ASSERT_EQUAL(0, image_ready_callback_2_timeout_counter);

  // 4
  // Clearing cache and expecting it not to find any images in cache and time
  // out
  reset_counters();
  uic_ota::clear_cache();

  file_exists = false;
  if (access("/tmp/UIC-000-000-010_EU_UIC-C87E6FB9-0010.dat", F_OK) == 0) {
    file_exists = true;
  }

  TEST_ASSERT_FALSE_MESSAGE(file_exists, "Did not remove cached file");

  uic_ota::get_by_unid("UIC-C87E6FB9-0010",
                       "UIC-000-000-010_EU",
                       image_ready_callback_1);
  contiki_test_helper_run(image_timeout * CLOCK_SECOND);
  TEST_ASSERT_EQUAL(1, image_ready_callback_1_counter);
  TEST_ASSERT_EQUAL(0, image_ready_callback_2_counter);
  TEST_ASSERT_EQUAL(1, image_ready_callback_1_timeout_counter);
  TEST_ASSERT_EQUAL(0, image_ready_callback_2_timeout_counter);
}
}
