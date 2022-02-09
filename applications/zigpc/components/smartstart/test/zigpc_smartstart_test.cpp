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

#include <workaround.hpp>

#include "unity.h"
#include "contiki_test_helper.h"

#include "zigpc_common_unid.h"
#include "zigpc_smartstart_fixt.h"
#include "zigpc_smartstart_int.hpp"

extern "C" {

#include "zigpc_common_zigbee_mock.h"
#include "zigpc_gateway_mock.h"
#include "zigpc_net_mgmt_mock.h"
#include "zigpc_net_mgmt_notify_mock.h"
#include "uic_mqtt_mock.h"

zigbee_eui64_t pc_eui64 = {0x0, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8};
size_t unid_len = ZIGBEE_EUI64_HEX_STR_LENGTH + strlen(ZIGPC_UNID_PREFIX);
const char *pc_unid_str = "zb-0002030405060708";

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp() {}

void tearDown() {}

/**
 * @brief Helper function to build Smartstart entry JSON objects
 *
 * @param dsk
 * @param include
 * @param pc_unid
 * @param unid
 * @return std::string JSON representation
 */
std::string helper_build_ssl_entry(std::string dsk,
                                   bool include,
                                   std::string pc_unid,
                                   std::string unid)
{
  return R"({"DSK":")" + dsk + R"(","Include":)" + (include ? "true" : "false")
         + R"(,"ProtocolControllerUnid":")" + pc_unid + R"(","Unid":")" + unid
         + R"(","PreferredProtocols":[]})";
}

void helper_expect_single_list_update(zigbee_eui64_t *eui64, char *eui64_str)
{
  zigbee_eui64_to_str_ExpectAndReturn(*eui64,
                                      NULL,
                                      strlen(eui64_str) + 1,
                                      SL_STATUS_OK);
  zigbee_eui64_to_str_IgnoreArg_str_buf();
  zigbee_eui64_to_str_ReturnArrayThruPtr_str_buf(eui64_str,
                                                 strlen(eui64_str) + 1);

  zigbee_eui64_to_str_ExpectAndReturn(*eui64,
                                      NULL,
                                      strlen(eui64_str) + 1,
                                      SL_STATUS_OK);
  zigbee_eui64_to_str_IgnoreArg_str_buf();
  zigbee_eui64_to_str_ReturnArrayThruPtr_str_buf(eui64_str,
                                                 strlen(eui64_str) + 1);
}

/**
 * @brief Testing SmartStart Manager Initialization
 *
 */
void test_smartstart_setup_fixtures(void)
{
  // ARRANGE
  zigpc_net_mgmt_register_observer_ExpectAndReturn(
    ZIGPC_NET_MGMT_NOTIFY_NETWORK_INIT,
    NULL,
    SL_STATUS_OK);
  zigpc_net_mgmt_register_observer_IgnoreArg_callback();
  zigpc_net_mgmt_register_observer_ExpectAndReturn(
    ZIGPC_NET_MGMT_NOTIFY_NODE_ADDED,
    NULL,
    SL_STATUS_OK);
  zigpc_net_mgmt_register_observer_IgnoreArg_callback();
  zigpc_net_mgmt_register_observer_ExpectAndReturn(
    ZIGPC_NET_MGMT_NOTIFY_NODE_REMOVED,
    NULL,
    SL_STATUS_OK);
  zigpc_net_mgmt_register_observer_IgnoreArg_callback();

  // ACT
  sl_status_t status = zigpc_smartstart_init_observers();

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}

void test_smartstart_init_on_net_init(void)
{
  // ARRANGE
  struct zigpc_net_mgmt_on_network_init data;
  zigbee_eui64_uint_t eui64_u = 0x0002030405060708;

  memcpy(data.pc_eui64, pc_eui64, sizeof(zigbee_eui64_t));

  zigbee_eui64_to_uint_ExpectAndReturn(data.pc_eui64, eui64_u);

  uic_mqtt_subscribe_Expect("ucl/SmartStart/List", NULL);
  uic_mqtt_subscribe_IgnoreArg_callback();

  // ACT
  zigpc_smartstart_on_network_init(&data);

  // ASSERT
}

void test_smartstart_manager_should_accept_empty_list_updates(void)
{
  // ARRANGE
  std::string ssl_str = "{\"value\":[]}";

  // ACT
  smartstart::Management::get_instance()->update_smartstart_cache(ssl_str);

  // ASSERT
}

void test_smartstart_manager_should_skip_adding_entry_on_invalid_dsk(void)
{
  // ARRANGE
  std::string ssl_str
    = "{\"value\":["
      + helper_build_ssl_entry("GHZ=3^#$#!#*?()/?!", true, "", "") + "]}";

  // ACT
  smartstart::Management::get_instance()->update_smartstart_cache(ssl_str);

  // ASSERT
}

void test_smartstart_manager_should_skip_adding_entry_on_incompatible_zwave_dsk(
  void)
{
  // ARRANGE
  std::string ssl_str = "{\"value\":["
                        + helper_build_ssl_entry(
                          "12345-64107-54321-12845-60475-62452-11111-59863",
                          true,
                          "",
                          "")
                        + "]}";

  // ACT
  smartstart::Management::get_instance()->update_smartstart_cache(ssl_str);

  // ASSERT
}

void test_smartstart_manager_should_skip_adding_entry_on_false_inclusion(void)
{
  // ARRANGE
  std::string ssl_str = "{\"value\":["
                        + helper_build_ssl_entry(
                          "00-11-02-03-44-AB-66-77-AA-BB-CC-0D-00-FF-12-34",
                          false,
                          "",
                          "")
                        + "]}";

  // ACT
  smartstart::Management::get_instance()->update_smartstart_cache(ssl_str);

  // ASSERT
}

void test_smartstart_manager_should_skip_adding_entry_on_unmatching_pc_unid(
  void)
{
  // ARRANGE
  std::string ssl_str = "{\"value\":["
                        + helper_build_ssl_entry(
                          "00-11-02-03-44-AB-66-77-AA-BB-CC-0D-00-FF-12-34",
                          true,
                          "1122334455667788",
                          "")
                        + "]}";

  // ACT
  smartstart::Management::get_instance()->update_smartstart_cache(ssl_str);

  // ASSERT
}

void test_smartstart_manager_should_skip_adding_entry_on_filled_device_unid(
  void)
{
  // ARRANGE
  std::string ssl_str = "{\"value\":["
                        + helper_build_ssl_entry(
                          "00-11-02-03-44-AB-66-77-AA-BB-CC-0D-00-FF-12-34",
                          true,
                          "",
                          "93243242")
                        + "]}";

  // ACT
  smartstart::Management::get_instance()->update_smartstart_cache(ssl_str);

  // ASSERT
}
void test_smartstart_manager_should_add_entry_on_matching_pc_unid(void)
{
  // ARRANGE
  std::string ssl_str = "{\"value\":["
                        + helper_build_ssl_entry(
                          "11-11-22-00-34-12-33-78-AA-BB-CC-0D-00-FF-12-34",
                          true,
                          pc_unid_str,
                          "")
                        + "]}";
  zigbee_eui64_t entry_eui64 = {0x11, 0x11, 0x22, 0x00, 0x34, 0x12, 0x33, 0x78};
  char entry_eui64_str[]     = "1111220034123378";
  zigbee_install_code_t entry_install_code
    = {0xAA, 0xBB, 0xCC, 0x0D, 0x00, 0xFF, 0x12, 0x34};

  zigpc_gateway_install_code_is_valid_ExpectAndReturn(entry_install_code,
                                                      8,
                                                      true);
  helper_expect_single_list_update(&entry_eui64, entry_eui64_str);

  zigpc_net_mgmt_add_node_ExpectWithArrayAndReturn(entry_eui64,
                                                   sizeof(zigbee_eui64_t),
                                                   entry_install_code,
                                                   8,
                                                   8,
                                                   SL_STATUS_OK);

  zigpc_netmgmt_network_permit_joins_ExpectAndReturn(true, SL_STATUS_OK);

  // ACT
  smartstart::Management::get_instance()->update_smartstart_cache(ssl_str);

  // ASSERT
}

void test_smartstart_manager_should_add_entry_empty_pc_unid(void)
{
  // ARRANGE
  std::string ssl_str = "{\"value\":["
                        + helper_build_ssl_entry(
                          "00-11-02-03-44-AB-66-77-AA-BB-CC-0D-00-FF-12-34",
                          true,
                          "",
                          "")
                        + "]}";
  zigbee_eui64_t entry_eui64 = {0x00, 0x11, 0x2, 0x3, 0x44, 0xAB, 0x66, 0x77};
  char entry_eui64_str[]     = "0011020344AB6677";
  zigbee_install_code_t entry_install_code
    = {0xAA, 0xBB, 0xCC, 0x0D, 0x00, 0xFF, 0x12, 0x34};

  zigpc_gateway_install_code_is_valid_ExpectAndReturn(entry_install_code,
                                                      8,
                                                      true);
  helper_expect_single_list_update(&entry_eui64, entry_eui64_str);

  zigpc_net_mgmt_add_node_ExpectWithArrayAndReturn(entry_eui64,
                                                   sizeof(zigbee_eui64_t),
                                                   entry_install_code,
                                                   8,
                                                   8,
                                                   SL_STATUS_OK);

  zigpc_netmgmt_network_permit_joins_ExpectAndReturn(true, SL_STATUS_OK);

  // ACT
  smartstart::Management::get_instance()->update_smartstart_cache(ssl_str);

  // ASSERT
}

void test_smartstart_manager_should_not_open_network_on_invalid_dsk(void)
{
  // ARRANGE
  std::string ssl_str = "{\"value\":["
                        + helper_build_ssl_entry(
                          "00-11-02-03-44-AB-66-77-AA-BB-F8-0D-00-FF-12-34",
                          true,
                          "",
                          "")
                        + "]}";
  zigbee_install_code_t entry_install_code
    = {0xAA, 0xBB, 0xF8, 0x0D, 0x00, 0xFF, 0x12, 0x34};

  zigpc_gateway_install_code_is_valid_ExpectAndReturn(entry_install_code,
                                                      8,
                                                      false);

  // ACT
  smartstart::Management::get_instance()->update_smartstart_cache(ssl_str);

  // ASSERT
}

/**
 * @brief The following scenario tests when two SmartStart entries are added
 * with different DSK's that has the same EUI64 but different install code.
 * In this situation, both entries will be used to perform node addition.
 *
 */
void test_smartstart_manager_should_add_both_entries_with_different_install_codes(
  void)
{
  // ARRANGE
  std::string ssl_str = "{\"value\":["
                        + helper_build_ssl_entry(
                          "AA-11-02-03-44-AB-66-77-AA-BB-CC-0D-00-FF-12-34",
                          true,
                          "",
                          "")
                        + ","
                        + helper_build_ssl_entry(
                          "AA-11-02-03-44-AB-66-77-34-23-AA-00-32-DF-42-44",
                          true,
                          "",
                          "")
                        + "]}";
  zigbee_eui64_t entry_eui64 = {0xAA, 0x11, 0x2, 0x3, 0x44, 0xAB, 0x66, 0x77};
  char entry_eui64_str[]     = "AA11020344AB6677";
  zigbee_install_code_t test_entry1_install_code
    = {0x34, 0x23, 0xAA, 0x00, 0x32, 0xDF, 0x42, 0x44};
  zigbee_install_code_t test_entry2_install_code
    = {0xAA, 0xBB, 0xCC, 0x0D, 0x00, 0xFF, 0x12, 0x34};

  zigpc_gateway_install_code_is_valid_ExpectAndReturn(test_entry1_install_code,
                                                      8,
                                                      true);
  helper_expect_single_list_update(&entry_eui64, entry_eui64_str);

  zigpc_net_mgmt_add_node_ExpectWithArrayAndReturn(entry_eui64,
                                                   sizeof(zigbee_eui64_t),
                                                   test_entry1_install_code,
                                                   8,
                                                   8,
                                                   SL_STATUS_OK);

  zigpc_gateway_install_code_is_valid_ExpectAndReturn(test_entry2_install_code,
                                                      8,
                                                      true);
  helper_expect_single_list_update(&entry_eui64, entry_eui64_str);

  zigpc_net_mgmt_add_node_ExpectWithArrayAndReturn(entry_eui64,
                                                   sizeof(zigbee_eui64_t),
                                                   test_entry2_install_code,
                                                   8,
                                                   8,
                                                   SL_STATUS_OK);

  zigpc_netmgmt_network_permit_joins_ExpectAndReturn(true, SL_STATUS_OK);

  // ACT
  smartstart::Management::get_instance()->update_smartstart_cache(ssl_str);

  // ASSERT
}

/**
 * @brief The following scenario tests when two SmartStart entries are added
 * with different DSK's that has the same EUI64 but one valid and one invalid
 * install code.
 *
 */
void test_smartstart_manager_should_add_one_valid_entry_with_multiple_dsk_adds(
  void)
{
  // ARRANGE
  std::string ssl_str = "{\"value\":["
                        + helper_build_ssl_entry(
                          "BB-11-02-03-44-AB-66-77-AA-BB-CC-0D-00-FF-12-34",
                          true,
                          "",
                          "")
                        + ","
                        + helper_build_ssl_entry(
                          "BB-11-02-03-44-AB-66-77-34-23-AA-00-32-DF-42-44",
                          true,
                          "",
                          "")
                        + "]}";
  zigbee_eui64_t entry_eui64 = {0xBB, 0x11, 0x2, 0x3, 0x44, 0xAB, 0x66, 0x77};
  char entry_eui64_str[]     = "BB11020344AB6677";
  zigbee_install_code_t test_entry1_install_code
    = {0x34, 0x23, 0xAA, 0x00, 0x32, 0xDF, 0x42, 0x44};
  zigbee_install_code_t test_entry2_install_code
    = {0xAA, 0xBB, 0xCC, 0x0D, 0x00, 0xFF, 0x12, 0x34};

  zigpc_gateway_install_code_is_valid_ExpectAndReturn(test_entry1_install_code,
                                                      8,
                                                      true);
  helper_expect_single_list_update(&entry_eui64, entry_eui64_str);

  zigpc_net_mgmt_add_node_ExpectWithArrayAndReturn(entry_eui64,
                                                   sizeof(zigbee_eui64_t),
                                                   test_entry1_install_code,
                                                   8,
                                                   8,
                                                   SL_STATUS_OK);

  zigpc_gateway_install_code_is_valid_ExpectAndReturn(test_entry2_install_code,
                                                      8,
                                                      false);

  zigpc_netmgmt_network_permit_joins_ExpectAndReturn(true, SL_STATUS_OK);

  // ACT
  smartstart::Management::get_instance()->update_smartstart_cache(ssl_str);

  // ASSERT
}

void test_smartstart_manager_should_not_update_non_existent_node_add(void)
{
  // ARRANGE
  struct zigpc_net_mgmt_on_node_added node_added
    = {.eui64 = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
  char entry_eui64_str[] = "00FFFFFFFFFFFFFF";

  zigbee_eui64_to_str_ExpectAndReturn(node_added.eui64,
                                      NULL,
                                      strlen(entry_eui64_str) + 1,
                                      SL_STATUS_OK);
  zigbee_eui64_to_str_IgnoreArg_str_buf();
  zigbee_eui64_to_str_ReturnArrayThruPtr_str_buf(entry_eui64_str,
                                                 strlen(entry_eui64_str) + 1);

  // ACT
  zigpc_smartstart_on_node_added(&node_added);

  // ASSERT
}

void test_smartstart_manager_should_update_entry_on_node_added(void)
{
  // ARRANGE
  std::string ssl_update_msg
    = R"({"DSK":"FC-11-02-03-44-AB-66-77-AA-BB-CC-0D-00-FF-12-34","Unid":"zb-FC11020344AB6677"})";
  struct zigpc_net_mgmt_on_node_added node_added
    = {.eui64 = {0xFC, 0x11, 0x2, 0x3, 0x44, 0xAB, 0x66, 0x77}};
  char entry_eui64_str[]      = "FC11020344AB6677";
  zigbee_eui64_uint_t eui64_u = 0xFC11020344AB6677;

  {  // Update SLL cache in common component
    std::string ssl_str = "{\"value\":["
                          + helper_build_ssl_entry(
                            "FC-11-02-03-44-AB-66-77-AA-BB-CC-0D-00-FF-12-34",
                            true,
                            "",
                            "")
                          + "]}";

    zigpc_gateway_install_code_is_valid_ExpectAndReturn(nullptr, 8, true);
    zigpc_gateway_install_code_is_valid_IgnoreArg_install_code();

    helper_expect_single_list_update(&node_added.eui64, entry_eui64_str);

    zigpc_net_mgmt_add_node_ExpectAndReturn(node_added.eui64,
                                            nullptr,
                                            8,
                                            SL_STATUS_OK);
    zigpc_net_mgmt_add_node_IgnoreArg_install_code();

    zigpc_netmgmt_network_permit_joins_ExpectAndReturn(true, SL_STATUS_OK);

    smartstart::Management::get_instance()->update_smartstart_cache(ssl_str);
  }

  // Behaviour for finding DSK and publishing SSL update
  zigbee_eui64_to_uint_ExpectAndReturn(node_added.eui64, eui64_u);
  helper_expect_single_list_update(&node_added.eui64, entry_eui64_str);

  uic_mqtt_publish_Expect("ucl/SmartStart/List/Update",
                          ssl_update_msg.c_str(),
                          ssl_update_msg.length(),
                          false);

  // ACT
  zigpc_smartstart_on_node_added(&node_added);

  // ASSERT
}

void test_smartstart_manager_should_not_update_non_existent_node_remove(void)
{
  // ARRANGE
  zigpc_net_mgmt_on_node_removed_t node_removed
    = {.eui64 = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
  zigbee_eui64_uint_t eui64_u = 0x00FFFFFFFFFFFFFF;

  zigbee_eui64_to_uint_ExpectAndReturn(node_removed.eui64, eui64_u);

  // ACT
  zigpc_smartstart_on_node_removed(&node_removed);

  // ASSERT
}

void test_smartstart_manager_should_update_entry_on_node_removed(void)
{
  // ARRANGE
  std::string ssl_update_msg
    = R"({"DSK":"00-11-02-03-44-AB-66-77-AA-BB-CC-0D-00-FF-12-34","Unid":""})";
  zigpc_net_mgmt_on_node_removed_t node_removed
    = {.eui64 = {0x00, 0x11, 0x2, 0x3, 0x44, 0xAB, 0x66, 0x77}};
  zigbee_eui64_uint_t eui64_u = 0x0011020344AB6677;
  char entry_eui64_str[]      = "0011020344AB6677";
  std::string entry_unid_str  = "zb-" + std::string(entry_eui64_str);

  {  // Update SLL cache in common component
    std::string ssl_str = "{\"value\":["
                          + helper_build_ssl_entry(
                            "00-11-02-03-44-AB-66-77-AA-BB-CC-0D-00-FF-12-34",
                            true,
                            "",
                            entry_unid_str)
                          + "]}";

    smartstart::Management::get_instance()->update_smartstart_cache(ssl_str);
  }

  // Behaviour for finding DSK and publishing SSL update
  zigbee_eui64_to_uint_ExpectAndReturn(node_removed.eui64, eui64_u);

  uic_mqtt_publish_Expect("ucl/SmartStart/List/Update",
                          ssl_update_msg.c_str(),
                          ssl_update_msg.length(),
                          false);

  // ACT
  zigpc_smartstart_on_node_removed(&node_removed);

  // ASSERT
}

void test_smartstart_manager_should_shutdown_properly(void)
{
  TEST_ASSERT_EQUAL(0, zigpc_smartstart_shutdown());
}

}  // extern "C"
