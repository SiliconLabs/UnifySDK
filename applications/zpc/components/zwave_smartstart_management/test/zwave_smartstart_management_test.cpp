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
#include "sl_log.h"
#include "workaround.hpp"
#include "smartstart_management.hpp"

// Contiki
#include "contiki_test_helper.h"
#include "sys/process.h"

#define LOG_TAG "zwave_smartstart_management_test"

extern std::string received_dsk;
extern std::string received_device_unid;
extern bool notify_node_added_called;
extern bool notify_node_removed_called;

using namespace smartstart;

extern "C" {
#include "zpc_config_mock.h"
#include "zwave_controller_callbacks_mock.h"
#include "zwave_network_management_mock.h"
#include "zwave_smartstart_management_fixt.h"
#include "zwave_controller_types.h"
#include "zwave_unid.h"
#include "s2_keystore.h"

int ctimer_expired(struct ctimer *c)
{
  return 1;
}

// clang-format off
// Device 4
zwave_dsk_t device_4_dsk_array = {0x76, 0xF7, 0xFE, 0xA7, 0xA7, 0xCA, 0x39, 0xF7
                                , 0xC8, 0x7F, 0xFD, 0x5F, 0x88, 0xE9, 0x14, 0xCA};
std::string device_4_dsk_str = "30455-65191-42954-14839-51327-64863-35049-05322";
std::string device_4_unid = "zw-C87FFD5F-0004";
// Device 6
zwave_dsk_t device_6_dsk_array = {0x2B, 0x99, 0x38, 0xC4, 0x70, 0x2A, 0x8C, 0x15,
                                  0x0A, 0x82, 0xC3, 0xB5, 0xBB, 0x52, 0x98, 0x64};

zwave_dsk_t device_7_dsk_array = {0x2B, 0x99, 0x38, 0xC4, 0x70, 0x2A, 0x8C, 0x15,
                                  0x0A, 0x82, 0xC3, 0xB5, 0xBB, 0x52, 0x0F, 0x48};

zwave_dsk_t device_8_dsk_array = {0x2B, 0x99, 0x38, 0xC4, 0x70, 0x2A, 0x8C, 0x15,
                                  0x0A, 0x82, 0xC3, 0xB3, 0xBB, 0x52, 0x0F, 0x47};

zwave_dsk_t device_9_dsk_array = {0x2B, 0x99, 0x38, 0xC4, 0x70, 0x2A, 0x8C, 0x15,
                                  0x0A, 0x82, 0xC3, 0xB1, 0xBB, 0x52, 0x0F, 0x46};
// clang-format on

static zwave_home_id_t home_id              = 0xC87FFD5F;
static zwave_home_id_t nwi_home_id          = 0xC87FFD5E;
static zwave_home_id_t device_6_nwi_home_id = 0xCA82C3B4;
static zwave_home_id_t device_7_nwi_home_id = 0xCA82C3B4;
static zwave_home_id_t device_8_nwi_home_id = 0xCA82C3B2;
static zwave_home_id_t device_9_nwi_home_id = 0xCA82C3B0;
static zpc_config_t test_config             = {};

const zwave_controller_callbacks_t *smartstart_callbacks;

static sl_status_t
  my_smartstart_callback_save(const zwave_controller_callbacks_t *cb, int n)
{
  smartstart_callbacks = cb;
  return SL_STATUS_OK;
}

static void add_device_4()
{
  // clang-format off
  std::string smartstart_list = "[{\"DSK\":\"30455-65191-42954-14839-51327-64863-35049-05322\""
                                 ",\"Include\":true"
                                 ",\"ProtocolControllerUnid\":\"zw-C87FFD5F-0001\""
                                 ",\"Unid\":\"\"}]";
  // clang-format on
  Management::get_instance()->update_smartstart_cache(smartstart_list);
}

static void add_device_5()
{
  // clang-format off
  std::string smartstart_list = "[{\"DSK\":\"30455-65191-42954-14839-51327-64863-35049-05311\""
                                 ",\"Include\":false"
                                 ",\"ProtocolControllerUnid\":\"zw-C87FFD5F-0001\""
                                 ",\"Unid\":\"\"}]";
  // clang-format on

  Management::get_instance()->update_smartstart_cache(smartstart_list);
}

static void add_device_6()
{
  // clang-format off
  std::string smartstart_list = "[{\"DSK\":\"11161-14532-28714-35861-02690-50101-47954-39012\""
                                 ",\"Include\":true"
                                 ",\"ProtocolControllerUnid\":\"\""
                                 ",\"PreferredProtocols\":[\"Z-Wave\",\"Z-Wave Long Range\"]"
                                 ",\"Unid\":\"\"}]";
  // clang-format on

  Management::get_instance()->update_smartstart_cache(smartstart_list);
}

static void add_device_7()
{
  // clang-format off
  std::string smartstart_list = "[{\"DSK\":\"11161-14532-28714-35861-02690-50101-47954-03912\""
                                 ",\"Include\":true"
                                 ",\"ProtocolControllerUnid\":\"\""
                                 ",\"PreferredProtocols\":[\"Z-Wave\",\"Z-Wave Long Range\"]"
                                 ",\"Unid\":\"\"}]";
  // clang-format on
  Management::get_instance()->update_smartstart_cache(smartstart_list);
}

static void add_device_8()
{
  // clang-format off
  std::string smartstart_list = "[{\"DSK\":\"11161-14532-28714-35861-02690-50099-47954-03911\""
                                 ",\"Include\":true"
                                 ",\"ProtocolControllerUnid\":\"\""
                                 ",\"PreferredProtocols\":[\"Z-Wave Long Range\",\"Z-Wave\"]"
                                 ",\"Unid\":\"\"}]";
  // clang-format on
  Management::get_instance()->update_smartstart_cache(smartstart_list);
}

static void add_device_9()
{
  // clang-format off
  std::string smartstart_list = "[{\"DSK\":\"11161-14532-28714-35861-02690-50097-47954-03910\""
                                 ",\"Include\":true"
                                 ",\"ProtocolControllerUnid\":\"\""
                                 ",\"PreferredProtocols\":[\"Z-Wave\"]"
                                 ",\"Unid\":\"\"}]";
  // clang-format on
  Management::get_instance()->update_smartstart_cache(smartstart_list);
}

////////////////////////////////////////////////////////////////////////////////
// Setup/Teardown
////////////////////////////////////////////////////////////////////////////////

// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

extern struct process zwave_smartstart_management_process;
void setUp()
{
  zwave_unid_set_home_id(home_id);
  zwave_network_management_get_node_id_IgnoreAndReturn(1);
  zwave_controller_register_callbacks_AddCallback(my_smartstart_callback_save);
  zwave_controller_register_callbacks_ExpectAndReturn(NULL, SL_STATUS_OK);
  zwave_controller_register_callbacks_IgnoreArg_callbacks();
  zwave_smartstart_management_setup_fixt();

  zpc_get_config_IgnoreAndReturn(&test_config);
}
void tearDown() {}
////////////////////////////////////////////////////////////////////////////////
// Tests
////////////////////////////////////////////////////////////////////////////////
void test_zwave_smartstart_on_inclusion_request()
{
  // Z-Wave, then Z-Wave Long Range for this test
  test_config.inclusion_protocol_preference = "1,2";
  // Inclusion request with Home ID matched and entry found
  zwave_network_management_smart_start_enable_Expect(true);
  add_device_4();
  zwave_network_management_add_node_with_dsk_ExpectAndReturn(device_4_dsk_array,
                                                             KEY_CLASS_ALL,
                                                             PROTOCOL_ZWAVE,
                                                             SL_STATUS_OK);

  smartstart_callbacks->on_smart_start_inclusion_request(nwi_home_id,
                                                         false,
                                                         NULL,
                                                         PROTOCOL_ZWAVE);

  smartstart_callbacks->on_smart_start_inclusion_request(nwi_home_id,
                                                         false,
                                                         NULL,
                                                         PROTOCOL_ZWAVE);
  // Inclusion request from already included node
  smartstart_callbacks->on_smart_start_inclusion_request(nwi_home_id,
                                                         true,
                                                         NULL,
                                                         PROTOCOL_ZWAVE);

  // Received inclusion request but don't want to include yet, disable SmartStart
  zwave_network_management_smart_start_enable_Expect(false);
  add_device_5();
  smartstart_callbacks->on_smart_start_inclusion_request(nwi_home_id,
                                                         false,
                                                         NULL,
                                                         PROTOCOL_ZWAVE);
}

void test_zwave_smartstart_inclusion_request_but_no_matching_protocol()
{
  // Z-Wave Long Range only for this test
  test_config.inclusion_protocol_preference = "1";

  // Inclusion request with Home ID matched and entry found
  zwave_network_management_smart_start_enable_Expect(true);
  add_device_4();
  // Not the preferred protocol will be ignored
  smartstart_callbacks->on_smart_start_inclusion_request(
    nwi_home_id,
    false,
    NULL,
    PROTOCOL_ZWAVE_LONG_RANGE);

  smartstart_callbacks->on_smart_start_inclusion_request(
    nwi_home_id,
    false,
    NULL,
    PROTOCOL_ZWAVE_LONG_RANGE);
}

void test_zwave_smartstart_inclusion_request_pick_protocol_from_config()
{
  // The SmartStart list prevails for this test
  test_config.inclusion_protocol_preference = "2,1";

  // Inclusion request with Home ID matched and entry found
  zwave_network_management_smart_start_enable_Expect(true);
  add_device_6();

  // Not the preferred protocol will be ignored
  smartstart_callbacks->on_smart_start_inclusion_request(
    device_6_nwi_home_id,
    false,
    NULL,
    PROTOCOL_ZWAVE_LONG_RANGE);

  // preferred protocol will be accepted
  zwave_network_management_add_node_with_dsk_ExpectAndReturn(device_6_dsk_array,
                                                             KEY_CLASS_ALL,
                                                             PROTOCOL_ZWAVE,
                                                             SL_STATUS_OK);
  smartstart_callbacks->on_smart_start_inclusion_request(device_6_nwi_home_id,
                                                         false,
                                                         NULL,
                                                         PROTOCOL_ZWAVE);
}

void test_zwave_smartstart_inclusion_request_pick_protocol_from_smartstart_list()
{
  // Inclusion request with Home ID matched and entry found
  zwave_network_management_smart_start_enable_Expect(true);
  add_device_7();

  smartstart_callbacks->on_smart_start_inclusion_request(device_7_nwi_home_id,
                                                         false,
                                                         NULL,
                                                         PROTOCOL_ZWAVE);

  smartstart_callbacks->on_smart_start_inclusion_request(
    device_7_nwi_home_id,
    false,
    NULL,
    PROTOCOL_ZWAVE_LONG_RANGE);

  zwave_network_management_add_node_with_dsk_ExpectAndReturn(device_7_dsk_array,
                                                             KEY_CLASS_ALL,
                                                             PROTOCOL_ZWAVE,
                                                             SL_STATUS_OK);

  smartstart_callbacks->on_smart_start_inclusion_request(device_7_nwi_home_id,
                                                         false,
                                                         NULL,
                                                         PROTOCOL_ZWAVE);

  zwave_network_management_smart_start_enable_Expect(true);
  add_device_8();
  smartstart_callbacks->on_smart_start_inclusion_request(device_8_nwi_home_id,
                                                         false,
                                                         NULL,
                                                         PROTOCOL_ZWAVE);
  zwave_network_management_add_node_with_dsk_ExpectAndReturn(
    device_8_dsk_array,
    KEY_CLASS_ALL,
    PROTOCOL_ZWAVE_LONG_RANGE,
    SL_STATUS_OK);

  smartstart_callbacks->on_smart_start_inclusion_request(
    device_8_nwi_home_id,
    false,
    NULL,
    PROTOCOL_ZWAVE_LONG_RANGE);

  zwave_network_management_smart_start_enable_Expect(true);
  // Z-Wave preferred only
  add_device_9();
  zwave_network_management_add_node_with_dsk_ExpectAndReturn(device_9_dsk_array,
                                                             KEY_CLASS_ALL,
                                                             PROTOCOL_ZWAVE,
                                                             SL_STATUS_OK);

  smartstart_callbacks->on_smart_start_inclusion_request(device_9_nwi_home_id,
                                                         false,
                                                         NULL,
                                                         PROTOCOL_ZWAVE);
}

void test_zwave_smartstart_inclusion_request_multiple_entires_in_list()
{
  // This test triggered bug UIC-917. Added to prevent regression
  // Z-Wave, then Z-Wave Long Range for this test
  test_config.inclusion_protocol_preference = "1,2";
  // Inclusion request with Home ID matched and entry found
  zwave_network_management_smart_start_enable_Expect(true);
  {
    // clang-format off
  std::string smartstart_list = "[{\"DSK\":\"52814-51459-08775-44249-17970-11704-09971-28891\""
                                 ",\"Include\":true"
                                 ",\"ProtocolControllerUnid\":\"\""
                                 ",\"Unid\":\"\"},"
                                 "{\"DSK\":\"62437-51085-11096-55529-33119-55593-41793-58876\""
                                 ",\"Include\":true"
                                 ",\"ProtocolControllerUnid\":\"\""
                                 ",\"Unid\":\"\"}]";
    // clang-format on
    Management::get_instance()->update_smartstart_cache(smartstart_list);
  }
  zwave_home_id_t nwi_2nd_entry = 0xC15FD928;
  // clang-format off
  zwave_dsk_t dsk_2nd_entry = {0xF3, 0xE5, 0xC7, 0x8D, 0x2B, 0x58, 0xD8, 0xE9,
                               0x81, 0x5F, 0xD9, 0x29, 0xA3, 0x41, 0xE5, 0xFC};
  // clang-format on
  zwave_network_management_add_node_with_dsk_ExpectAndReturn(dsk_2nd_entry,
                                                             KEY_CLASS_ALL,
                                                             PROTOCOL_ZWAVE,
                                                             SL_STATUS_OK);
  smartstart_callbacks->on_smart_start_inclusion_request(nwi_2nd_entry,
                                                         false,
                                                         NULL,
                                                         PROTOCOL_ZWAVE);

  // Inclusion request from already included node
  smartstart_callbacks->on_smart_start_inclusion_request(nwi_2nd_entry,
                                                         true,
                                                         NULL,
                                                         PROTOCOL_ZWAVE);
}

void test_zwave_smartstart_on_node_added()
{
  notify_node_added_called = false;
  smartstart_callbacks->on_node_added(SL_STATUS_OK,
                                      NULL,
                                      4,
                                      device_4_dsk_array,
                                      KEY_CLASS_ALL,
                                      ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE,
                                      PROTOCOL_ZWAVE);

  TEST_ASSERT_TRUE(notify_node_added_called == true);
  TEST_ASSERT_TRUE(received_dsk == device_4_dsk_str);
  TEST_ASSERT_TRUE(received_device_unid == device_4_unid);

  notify_node_added_called = false;
  smartstart_callbacks->on_node_added(SL_STATUS_FAIL,
                                      NULL,
                                      4,
                                      device_4_dsk_array,
                                      KEY_CLASS_ALL,
                                      ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE,
                                      PROTOCOL_ZWAVE);

  TEST_ASSERT_TRUE(notify_node_added_called == false);
}

void test_zwave_smartstart_on_node_deleted()
{
  notify_node_removed_called = false;
  smartstart_callbacks->on_node_deleted(4);
  TEST_ASSERT_TRUE(notify_node_removed_called == true);
  TEST_ASSERT_TRUE(received_device_unid == device_4_unid);

  notify_node_removed_called = false;
  smartstart_callbacks->on_node_deleted(0);
  TEST_ASSERT_TRUE(notify_node_removed_called == false);
}
}