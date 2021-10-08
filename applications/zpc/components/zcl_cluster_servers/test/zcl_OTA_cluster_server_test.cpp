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
#include "unity.h"
#include "attribute.hpp"
#include "attribute_store.h"
#include "datastore.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "attribute_store_defined_attribute_types.h"
#include "mqtt_mock_helper.h"
#include "../src/zcl_OTA_cluster_server.hpp"
#include "zwave_COMMAND_CLASS_MANUFACTURER_SPECIFIC_attribute_id.h"
#include "zwave_unid.h"
#include "workaround.h"
#include "zpc_config.h"
using namespace attribute_store;

attribute n5ep0;

extern "C" {
#include "zwave_command_class_firmware_update_mock.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  //Do some hacking
  zpc_config_t *cfg   = (zpc_config_t *)zpc_get_config();
  cfg->ota_cache_path = ".";
  cfg->ota_cache_size = 1024;

  datastore_init(":memory:");
  attribute_store_init();

  zcl_OTA_cluster_server_init();
  mqtt_mock_helper_init();

  //This is ugly....
  zwave_unid_set_home_id(0xDEADBEEF);

  n5ep0 = attribute::root()
            .emplace_node<zwave_home_id_t>(ATTRIBUTE_HOME_ID, 0x11111111)
            .emplace_node<zwave_node_id_t>(ATTRIBUTE_NODE_ID, 5)
            .emplace_node<zwave_endpoint_id_t>(ATTRIBUTE_ENDPOINT_ID, 0);

  n5ep0.emplace_node<uint32_t>(ATTRIBUTE_MANUFACTURER_SPECIFIC_MANUFACTURER_ID,
                               0);
  n5ep0.emplace_node<uint32_t>(ATTRIBUTE_MANUFACTURER_SPECIFIC_PRODUCT_TYPE_ID,
                               42);
  n5ep0.emplace_node<uint32_t>(ATTRIBUTE_MANUFACTURER_SPECIFIC_PRODUCT_ID, 1);

  n5ep0.emplace_node<uint32_t>(ATTRIBUTE_CC_VERSION_VERSION_REPORT_DATA, 1)
    .emplace_node<uint32_t>(ATTRIBUTE_CC_VERSION_FIRMWARE, 0x1)
    .emplace_node<uint32_t>(ATTRIBUTE_CC_VERSION_FIRMWARE_VERSION, 0x00112233);
  n5ep0.child_by_type(ATTRIBUTE_CC_VERSION_VERSION_REPORT_DATA).emplace_node<uint32_t>(ATTRIBUTE_CC_VERSION_HARDWARE_VERSION, 1);
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_zcl_OTA_cluster_server_current_version()
{
  TEST_ASSERT_GREATER_OR_EQUAL(
    1,
    mqtt_mock_helper_get_num_publish(
      "ucl/by-unid/zw-DEADBEEF-0005/ep0/OTA/Attributes/UIID/"
      "ZWave-0000-0001-002a-01-01/CurrentVersion/Reported"));
}

void test_zcl_OTA_cluster_server_test_image_available()
{
  attribute fwu_fw
    = n5ep0.emplace_node<uint32_t>(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU, 0x1)
        .emplace_node<uint32_t>(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW, 0x1);

  {
    // Valgrind complains that meta (std::string) can leak memory in this context.
    // We'll put it in its own scope to make sure it's deleted when we don't need it
    // anymore.
    std::string meta
      = R"xxx({"Version":"1.0.2","ApplyAfter":"2021-06-29T16:39:57+02:00","Filename":"ZW_SensorPir_7.16.1_104_EFR32ZG13P32_REGION_EU_DEBUG_v255.gbl"})xxx";
    mqtt_mock_helper_publish("ucl/OTA/info/ZWave-0000-0001-002a-01-01/all",
                             meta.c_str(),
                             meta.length());
    mqtt_mock_helper_publish(
      "ucl/OTA/info/ZWave-0000-0001-002a-01-01/zw-DEADBEEF-0005",
      meta.c_str(),
      meta.length());
  }

  TEST_ASSERT_EQUAL(
    1,
    mqtt_mock_helper_get_num_subscribers(
      "ucl/OTA/data/ZWave-0000-0001-002a-01-01/zw-DEADBEEF-0005"));

  TEST_ASSERT_GREATER_OR_EQUAL(
    1,
    mqtt_mock_helper_get_num_publish(
      "ucl/OTA/data/ZWave-0000-0001-002a-01-01/zw-DEADBEEF-0005/get"));

  fwu_fw.delete_node();

  // There is an issue when deleting an FU target, the problem is that we need the target id
  // to generated the UIID before we can unretain the MQTT subscriptions. The problem is that
  // the target id data has just been wiped. Maybe we should change the when the DELETED callback
  // is triggered ?
  TEST_IGNORE_MESSAGE("Deleted FU targets are not handled correctly");

  TEST_ASSERT_EQUAL(
    0,
    mqtt_mock_helper_get_num_subscribers(
      "ucl/OTA/data/ZWave-0000-0001-002a-01-01/zw-DEADBEEF-0005"));
}

void test_zcl_OTA_cluster_server_test_file_available()
{
  n5ep0.emplace_node<uint32_t>(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU, 0x1)
    .emplace_node<uint32_t>(ATTRIBUTE_COMMAND_CLASS_FWU_MD_FWU_FW, 0x1);
  std::string meta
    = R"xxx({"Version":"1.0.2","ApplyAfter":"2021-06-29T16:39:57+02:00","Filename":"ZW_SensorPir_7.16.1_104_EFR32ZG13P32_REGION_EU_DEBUG_v255.gbl"})xxx";

  mqtt_mock_helper_publish("ucl/OTA/info/ZWave-0000-0001-002a-01-01/all",
                           meta.c_str(),
                           meta.length());
  mqtt_mock_helper_publish(
    "ucl/OTA/info/ZWave-0000-0001-002a-01-01/zw-DEADBEEF-0005",
    meta.c_str(),
    meta.length());

  TEST_ASSERT_EQUAL(
    1,
    mqtt_mock_helper_get_num_subscribers(
      "ucl/OTA/data/ZWave-0000-0001-002a-01-01/zw-DEADBEEF-0005"));

  TEST_ASSERT_GREATER_OR_EQUAL(
    1,
    mqtt_mock_helper_get_num_publish(
      "ucl/OTA/data/ZWave-0000-0001-002a-01-01/zw-DEADBEEF-0005/get"));

  command_class_firmware_update_initiate_firmware_update_ExpectAndReturn(
    5,
    0,
    1,
    0,
    nullptr,
    SL_STATUS_OK);
  command_class_firmware_update_initiate_firmware_update_IgnoreArg_apply_after();
  command_class_firmware_update_initiate_firmware_update_IgnoreArg_filename();

  std::string data = "My firmware data";
  mqtt_mock_helper_publish(
    "ucl/OTA/data/ZWave-0000-0001-002a-01-01/zw-DEADBEEF-0005",
    data.c_str(),
    data.length());
}
}
